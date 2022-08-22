Contains a simple DeepState test harness that fuzzes inputs for a rule-given action (division). STest is used for the rule construct. Both libraries (DeepState and STest) are integrated. 

## Get started
Once you have a setup with DeepState configured and this repository cloned, `cd` into this directory and build the executable using `make`: (linked against DeepState's static library)
```sh
<username>@<systemname>:~/STest-Demo/Fuzz/DeepState$ make
g++ -Wall -g -std=c++11 -Iinclude -no-pie -ldeepstate -o fuzztest_deepstateharness.bin bsd_random.o Random.o Pick.o Pick_default.o fuzztest_deepstateharness.o
```
Run it through a fuzzer with desired arguments:
```sh
<username>@<systemname>:~/STest-Demo/Fuzz/DeepState$ ./fuzztest_deepstateharness.bin --fuzz --timeout=1  --fuzz_save_failing --output_test_dir out
INFO: Starting fuzzing
WARNING: No seed provided; using 1661044705
WARNING: No test specified, defaulting to first test defined (ArithmeticOps_DivisionByZero)
CRITICAL: fuzztest_deepstateharness.cpp(24): Divisor cannot be 0! (dividend and divisor values for this run: 94, 0)
ERROR: Failed: ArithmeticOps_DivisionByZero
INFO: Saved test case in file `out/60f5f7e58fb4283ccc6ed4baa07b80ae09300258.fail`
CRITICAL: fuzztest_deepstateharness.cpp(24): Divisor cannot be 0! (dividend and divisor values for this run: 3, 0)
ERROR: Failed: ArithmeticOps_DivisionByZero
INFO: Saved test case in file `out/848f90b1d99a1491324071faf2aa60ce44de58f2.fail`

...

CRITICAL: fuzztest_deepstateharness.cpp(24): Divisor cannot be 0! (dividend and divisor values for this run: 29, 0)
ERROR: Failed: ArithmeticOps_DivisionByZero
INFO: Saved test case in file `out/84dade03325d816bd685e529b6fccb34f0ecfef2.fail`
CRITICAL: fuzztest_deepstateharness.cpp(24): Divisor cannot be 0! (dividend and divisor values for this run: 52, 0)
ERROR: Failed: ArithmeticOps_DivisionByZero
INFO: Saved test case in file `out/cc0d63cab435e99435359b316833d3990966c207.fail`
CRITICAL: fuzztest_deepstateharness.cpp(24): Divisor cannot be 0! (dividend and divisor values for this run: 40, 0)
ERROR: Failed: ArithmeticOps_DivisionByZero
INFO: Saved test case in file `out/936256140c305b1eb25769e14c0e286bcce12f52.fail`
CRITICAL: fuzztest_deepstateharness.cpp(24): Divisor cannot be 0! (dividend and divisor values for this run: 17, 0)
ERROR: Failed: ArithmeticOps_DivisionByZero
INFO: Saved test case in file `out/c0843a650e1b72f46c51cc1f137051e116540b90.fail`
CRITICAL: fuzztest_deepstateharness.cpp(24): Divisor cannot be 0! (dividend and divisor values for this run: 52, 0)
ERROR: Failed: ArithmeticOps_DivisionByZero
INFO: Saved test case in file `out/f263dfa03b181888a475e934b96feaddc3192ba5.fail`
CRITICAL: fuzztest_deepstateharness.cpp(24): Divisor cannot be 0! (dividend and divisor values for this run: 34, 0)
ERROR: Failed: ArithmeticOps_DivisionByZero
INFO: Saved test case in file `out/b54132703e73fe81e42205208d739a45f962bd3f.fail`
INFO: Done fuzzing! Ran 498928 tests (498928 tests/second) with 4988 failed/493940 passed/0 abandoned tests
```
I'm using the default one here (other fuzzing engines would require the executable to be compiled in their own ways<sup>1</sup>) with a maximum runtime of one second and I'm saving the failing tests to a pre-created output directory called 'out'. Each of these will be recorded in a separate file (so the number of such files in that directory should be equal to the number of failing tests, i.e. `ls out/*.fail | wc -l` will return 4988 for my run above).

The files from each run will stack as more fuzzing sessions with the same arguments go by. Thus, you may want to remove those files and start collecting afresh (do something like `cd out && rm -rf *.fail`, which you can also add to my `makefile` under `clean:`, and then run `make` after `make clean` per session). Note that there won't be anything interesting in these files in this case and in general too for other test cases fuzzed through this default fuzzer, which just brute forces through possible input combinations.

It doesn't work differently from say, seeded rng calls (here, those calls would be to generate the dividend and the divisor in the harness that I created) over an infinite loop for going over possible values of a data type. For instance, if I would have used two `Symbolic<uint32_t>`/`symbolic_uint32_t`s for the numbers, going full range (0 to 4,294,967,295) would mean that the desired crash result (denominator/divisor equals 0) will not be found for a very long time, and this is primarily because each value in the supplied range occurs in an approximately equal manner via a cyclic routine (appears to be a linear congruential uniform approach, as used in most random number generators; and then STest uses a linear feedback shift register approach in [bsd_random.c](https://github.com/Anirban166/STest-Demo/blob/89431c3cf43e4b4fd379ad474289beb875b8ebc9/STest/Random/bsd_random.c#L56) - not much of a difference!)

One doesn't need to explicitly look at the code to learn this. For instance, you can observe this just by looking at the metrics of a few repeated runs here - the times the tests failed or the times zero occurred for the divisor from the total is roughly equal to one:(interval range within which the numbers are being generated). For instance, with a range of 0 to 100 that I [set](https://github.com/Anirban166/STest-Demo/blob/89431c3cf43e4b4fd379ad474289beb875b8ebc9/Fuzz/DeepState/fuzztest_deepstateharness.cpp#L23) for my divisor to have possible values from, that ratio would be near about 1:100, or in other words, a zero occurs per 100 generations.

Anyways, using this very basic fuzzer won't get us anywhere new where ordinary type-constrained randomized inputs would take one with a brute force progression. Instead, fuzzers which are able to generate/throw inputs based on some heuristics and progressively iterate the process to find unique/interesting ones are what's the lookout for in general.

One of them is AFL. It takes as input a set of valid and *presumably* interesting inputs (forms a dictionary), altogether known as the 'corpus'. The set of interesting inputs and test cases eventually increases, but you need something to start with, ideally :)

These 'interesting' inputs depend on a metric, which is code coverage in AFL and most other prominent fuzzers (libFuzzer for instance). Any input resulting in new code coverage is taken to be a path to consider for further mutation, and is of exploratory interest. This tends to be a form of self-learning for the fuzzer, but more so importantly, it enables the fuzzer to navigate its way around (as opposed to dumb fuzzers, although `-d` is a thing here :) input generation through active feedback based on this metric. Such guided heuristics enable the subsequent picking of inputs to be directed at finding new bugs/leaks/vulnerabilities from crashes (for instance, unique ones could be identified from a newfound crash signature from a stacktrace) and basically anything that triggers new behaviour and falls under the instrumentation that is defined. 

As for how these inputs get transformed themselves, it depends on both deterministic and probabilistic procedures such as bitflips (basically flipping of n-consecutive bits by XOR'ing them with others), algebraic manipulations, splicing, etc. These are the 'stages' of input mutation, which are repeated over and over again, with each stage possibly having thousands of iterations themselves (just a guess based on the iterations per second shown and the noticeable change in stages visually reflected in the 'stage progress' section of the interface when running the fuzzer).

AFL generates a queue and loads into it each test case/file (which gets distributed to processes branched from Master when forked, with synchronized updates to the queue) that is presented as initial input or corpus. These get mutated by the aforementioned (and others that I don't know of) strategies during the fuzzing process, and ones which result in a new state transition (as recorded by the instrumentation) get added to the queue.

A caveat here would be that this sort of fuzzing scales real quick (it is not a thorough search as something such as a symbolic execution engine would do as that misses the point here, but it is still quick to keep exploring the search space in a directed manner till eternity) and comes at the cost of millions of writes (apart from `forks()`, file I/O, and all that jazz) in a relatively short amount of time, which can drain your SSD (like `tmpfs`, keeping the output files in virtual memory could potentially help with this). I wish such computationally intensive workloads could be pushed into GPUs, but AFL only uses CPU cores!