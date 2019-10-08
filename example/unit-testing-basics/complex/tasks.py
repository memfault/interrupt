# Check out https://interrupt.memfault.com/blog/building-a-cli-for-firmware-projects
# for reference on writing Invoke tasks for firmware

from invoke import Collection, Config, task

@task
def test(ctx, coverage=False):
    """Run the unit tests"""
    env_dict = {'CPPUTEST_HOME': '/usr/local/Cellar/cpputest/3.8', 'TARGET_PLATFORM': ""}

    cmd = "make"
    if coverage:
        cmd += " lcov"

    with ctx.cd("tests"):
        ctx.run(cmd, env=env_dict)

ns = Collection(test)
config = Config(defaults={"run": {"pty": True}})
ns.configure(config)
