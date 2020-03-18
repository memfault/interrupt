from future.standard_library import install_aliases

install_aliases()

import click
import os
import subprocess
import psycopg2

from contextlib import contextmanager
from prettytable import PrettyTable
from urllib.parse import urlparse

DATABASE_URL = os.environ.get("DATABASE_URL", "postgresql://localhost:5432/codesizes")


class CodesizeData:
    def __init__(self, revision, build, parent_revision, text, data, bss, message):
        self.revision = revision
        self.build = build
        self.parent_revision = parent_revision
        self.text = text
        self.data = data
        self.bss = bss
        self.message = message

    def to_dict(self):
        return dict(
            revision=self.revision,
            build=self.build,
            parent_revision=self.parent_revision,
            text=self.text,
            data=self.data,
            bss=self.bss,
            message=self.message,
        )


@contextmanager
def db_conn(*args, **kwargs):
    url = urlparse(DATABASE_URL)
    dbname = url.path[1:]
    user = url.username
    password = url.password
    host = url.hostname
    port = url.port

    conn = psycopg2.connect(
        dbname=dbname, user=user, password=password, host=host, port=port
    )
    try:
        yield conn
    finally:
        conn.close()


def _upload_codesize(codesize):
    with db_conn() as conn:
        cursor = conn.cursor()
        cursor.execute(
            """
            INSERT INTO codesizes (revision, build, parent_revision, text, data, bss, message)
            VALUES ('{REVISION}', '{BUILD}', '{PARENT_REVISION}', {TEXT}, {DATA}, {BSS}, '{MESSAGE}')
            """.format(
                REVISION=codesize.revision,
                BUILD=codesize.build,
                PARENT_REVISION=codesize.parent_revision,
                TEXT=codesize.text,
                DATA=codesize.data,
                BSS=codesize.bss,
                MESSAGE=codesize.message,
            )
        )
        conn.commit()


def _fetch_codesize(build, revision="HEAD"):
    with db_conn() as conn:
        cursor = conn.cursor()
        cursor.execute(
            """SELECT revision, build, parent_revision, 
                      text, data, bss, message 
               FROM codesizes 
               WHERE build = '{}' 
                 AND revision = '{}'""".format(
                build, revision
            )
        )
        record = cursor.fetchone()
        revision, build, parent_revision, text, data, bss, message = record
        return CodesizeData(
            revision=revision,
            build=build,
            parent_revision=parent_revision,
            text=text,
            data=data,
            bss=bss,
            message=message,
        )


def _calculate_codesize(binary_path):
    output = subprocess.check_output(
        ["arm-none-eabi-size", binary_path], encoding="UTF-8"
    )
    # Get the second line
    size_output = output.splitlines()[1]

    # Pull out text, data, and bss
    text, data, bss, *_ = size_output.split()
    return (int(text), int(data), int(bss))


def _convert_revision_to_git_sha(revision):
    return subprocess.check_output(
        ["git", "rev-parse", "{}".format(revision)], encoding="UTF-8"
    ).strip()


@click.group()
def cli():
    pass


@cli.command()
@click.argument("binary", type=click.Path(exists=True))
def calculate(binary):
    """Calculate the code size of the given binary"""
    text, data, bss = _calculate_codesize(binary)
    print(dict(text=text, data=data, bss=bss))


@cli.command()
@click.argument("revision")
@click.option("--build", default="default")
def fetch(build, revision):
    """
    Fetch code size data for the given build and revision
    """
    revision = _convert_revision_to_git_sha(revision)
    codesize = _fetch_codesize(build, revision)
    if not codesize:
        click.echo("No codesize for build: {}, revision: {}".format(build, revision))
        return

    click.echo(dict(text=codesize.text, data=codesize.data, bss=codesize.bss))


@cli.command()
@click.argument("binary", type=click.Path(exists=True))
@click.option("--build", default="default")
@click.option("--revision", default="HEAD")
def stamp(binary, build, revision):
    """
    Calculates the size of the BINARY (.elf) provided and stamps the current
    git revision with the codesize for the given BUILD. 

    For example, if you wanted to calculate the size for `build/firmware.elf`
    and it is a `debug` build.

      $ python codesizes.py build/firmware.elf --build debug
    """
    text, data, bss = _calculate_codesize(binary)
    revision = _convert_revision_to_git_sha(revision)
    parent_revision = subprocess.check_output(
        ["git", "rev-parse", "{}^1".format(revision)], encoding="UTF-8"
    ).strip()
    message = subprocess.check_output(
        ["git", "log", "-1", "--pretty=%s", revision], encoding="UTF-8"
    ).strip()
    # git log -1 --pretty=%s $1

    # Upload!
    codesize = CodesizeData(
        revision=revision,
        build=build,
        parent_revision=parent_revision,
        text=text,
        data=data,
        bss=bss,
        message=message,
    )
    _upload_codesize(codesize)


@cli.command()
@click.argument("binary", type=click.Path(exists=True))
@click.option("--build", default="default")
@click.option("--revision")
def diff(binary, build, revision):
    """
    Calculate the size of the BINARY given, and compare it
    to either the REVISION provided or the "merge-base" of the
    current branch and master for the given BUILD.
    """
    text, data, bss = _calculate_codesize(binary)
    if not revision:
        click.echo("Using `git merge-base` against master branch")
        revision = subprocess.check_output(
            ["git", "merge-base", "HEAD", "master"], encoding="UTF-8"
        ).strip()

    codesize = _fetch_codesize(build, revision)
    if not codesize:
        click.echo("No codesize for build: {}, revision: {}".format(build, revision))
        return

    text_delta = text - codesize.text
    data_delta = data - codesize.data
    bss_delta = bss - codesize.bss

    t = PrettyTable()
    t.field_names = ["Region", ".text", ".data", ".bss"]
    t.add_row(["Local", text, data, bss])
    t.add_row(["Against", codesize.text, codesize.data, codesize.bss])
    t.add_row(["Delta", text_delta, data_delta, bss_delta])
    t.align = "r"

    click.echo("Comparing code size of current .elf against: {}".format(revision))
    click.echo(t)


@cli.command()
def db_init():
    """Create the database table on the configured database"""
    with db_conn() as conn:
        cursor = conn.cursor()
        cursor.execute(
            """
            CREATE TABLE "codesizes" (
                "created_at" timestamp NOT NULL DEFAULT NOW(),
                "revision" varchar NOT NULL,
                "build" varchar NOT NULL,
                "parent_revision" varchar NOT NULL,
                "text" int4 NOT NULL,
                "data" int4 NOT NULL,
                "bss" int4 NOT NULL,
                "message" varchar NOT NULL DEFAULT ''::character varying,
                PRIMARY KEY ("revision","build")
            )
            """
        )
        conn.commit()


def main():
    cli()


if __name__ == "__main__":
    main()
