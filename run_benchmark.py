import tempfile
import os
import json
import argparse
import subprocess
import csv
import glob


class Repository:
    def __init__(self, config: dict):
        self.dir = tempfile.mkdtemp(dir="/opt/workspace/tmp")
        self.url = config["repo_url"]
        self.commit = config["commit"]
        print(self.url)
        print(self.dir)

    def checkout(self, commit):
        self.commit = commit
        subprocess.run("git checkout".split() + [self.commit], check=True)

    def clone(self):
        print("clone")
        subprocess.run("git clone".split() + [self.url, self.dir], check=True)

    def find_path(self, filename):
        search_expression = self.dir + f"/**/{filename}"
        return os.path.dirname(glob.glob(search_expression, recursive=True)[0])


class DB_bench:
    def __init__(self, config: dict, repo: Repository, pmemkv_repo: Repository):
        self.path = repo.dir
        self.pmemkv_path = pmemkv_repo.dir
        self.pmemkv = pmemkv_repo
        self.run_output = None
        self.env = config["env"]
        self.benchmark_params = config["params"]

    def build(self):
        library_path = self.pmemkv.find_path("libpmemkv.so.1")
        include_path = self.pmemkv.find_path("libpmemkv.hpp")
        build_env = {
            "LIBRARY_PATH": library_path,
            "CPLUS_INCLUDE_PATH": include_path,
            "PATH": os.environ["PATH"],
        }
        print(f"{build_env}=")
        try:
            subprocess.run(
                "make bench".split(), env=build_env, cwd=self.path, check=True
            )
        except subprocess.CalledProcessError as e:
            print(f"Cannot build benchmark: {e}")
            raise e

    def run(self):
        try:
            env = self.env
            env["PATH"] = self.path + ":" + os.environ["PATH"]
            env["LD_LIBRARY_PATH"] = self.pmemkv.find_path("libpmemkv.so.1")
            print(f"{env=}")
            self.run_output = subprocess.run(
                ["pmemkv_bench", "--csv_output"] + self.benchmark_params,
                cwd=self.path,
                env=env,
                capture_output=True,
                check=True,
            )
        except subprocess.CalledProcessError as e:
            print(f"benchmark process failed: {e.output}: ")
            raise e

    def get_results(self):
        OutputReader = csv.DictReader(
            self.run_output.stdout.decode("UTF-8").split("\n"), delimiter=","
        )
        return [dict(x) for x in OutputReader]


class Pmemkv:
    def __init__(self, config: dict, repo: Repository):
        self.repo = repo
        self.path = self.repo.dir
        self.build_env = config["env"]
        self.build_env["PATH"] = os.environ["PATH"]
        self.cmake_params = config["cmake_params"]

    def build(self):
        cpus = f"{os.cpu_count()}"
        print(f"{self.build_env}=")
        try:
            subprocess.run(
                "cmake .".split() + self.cmake_params,
                env=self.build_env,
                cwd=self.path,
                check=True,
            )
            subprocess.run(
                ["make", "-j", cpus], env=self.build_env, cwd=self.path, check=True
            )
        except subprocess.CalledProcessError as e:
            print(f"Cannot build pmemkv: {e.output}")
            raise e
        pass


def print_results(results_dict):
    print(json.dumps(results_dict, indent=4, sort_keys=True))


if __name__ == "__main__":

    # TODO: cache builds
    parser = argparse.ArgumentParser(description="Runs pmemkv_bench")
    parser.add_argument("config_path", help="Path to json config file")
    args = parser.parse_args()
    print(args.config_path)

    config = None
    with open(args.config_path) as config_path:
        config = json.loads(config_path.read())
    print(config)
    db_bench_repo = Repository(config["db_bench"])
    db_bench_repo.clone()

    pmemkv_repo = Repository(config["pmemkv"])
    pmemkv_repo.clone()
    pmemkv = Pmemkv(config["pmemkv"], pmemkv_repo)
    pmemkv.build()

    benchmark = DB_bench(config["db_bench"], db_bench_repo, pmemkv_repo)

    benchmark.build()
    benchmark.run()
    benchmark_results = benchmark.get_results()
    print_results(benchmark_results)
