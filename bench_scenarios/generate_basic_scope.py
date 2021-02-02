import json
import itertools
import jsonschema

benchmarks = [
    "fillseq",
    "fillrandom",
    "fillseq,readrandom,readrandom",
    "fillrandom,readrandom,readrandom",
    "fillseq,readseq,readseq",
    "fillrandom,readseq,readseq",
    "readwhilewriting",
    "readrandomwriterandom",
]
size = [8, 128]
number_od_elements = 100000000


def concurrent_engines():

    number_of_threads = [1, 4, 8, 12, 18, 24]
    engine = ["cmap", "csmap"]

    result = itertools.product(benchmarks, size, number_of_threads, engine)
    return list(result)


def single_threaded_engines():
    number_of_threads = [1]
    engine = ["radix", "stree"]
    result = itertools.product(benchmarks, size, number_of_threads, engine)
    return list(result)


def generate():
    benchmarks = concurrent_engines()
    benchmarks.extend(single_threaded_engines())
    benchmarks_configuration = []
    db_path = "/mnt/pmem0/pmemkv-bench"
    for benchmark in benchmarks:
        benchmark_settings = {
            "env": {
                "NUMACTL_CPUBIND": f"file:{os.dirname(db_path)}",
            },
            "params": {
                "--benchmarks": f"{benchmark[0]}",
                "--value_size": f"{benchmark[1]}",
                "--threads": f"{benchmark[2]}",
                "--engine": f"{benchmark[3]}",
                "--num": f"{number_od_elements}",
                "--db": db_path,
                "--db_size_in_gb": "200",
            },
        }

        benchmarks_configuration.append(benchmark_settings)

    return benchmarks_configuration


if __name__ == "__main__":
    output = generate()
    schema = None
    with open("bench.schema.json", "r") as schema_file:
        schema = json.loads(schema_file.read())
    try:
        jsonschema.validate(instance=output, schema=schema)
    except jsonschema.exceptions.ValidationError as e:
        print(e)
        exit(1)
    print(json.dumps(output, indent=4))
