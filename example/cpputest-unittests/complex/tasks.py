import os, shutil, sys
from invoke import Collection, Config, Exit, task
from shutil import which

ROOT_DIR = os.path.dirname(__file__)

@task()
def test(ctx, lcov=False):
    """Spawn GDB and flash application & softdevice"""
    env_dict = {'CPPUTEST_HOME': '/usr/local/Cellar/cpputest/3.8', 'TARGET_PLATFORM': ""}

    with ctx.cd("tests"):
        ctx.run("make", env=env_dict)

# Add all tasks to the namespace
ns = Collection(test)
# Configure every task to act as a shell command (will print colors, allow interactive CLI)
# Add our extra configuration file for the project
config = Config(defaults={"run": {"pty": True}})
ns.configure(config)
