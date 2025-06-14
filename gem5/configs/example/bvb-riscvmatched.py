# Copyright (c) 2022 The Regents of the University of California
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met: redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer;
# redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution;
# neither the name of the copyright holders nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
"""
This gem5 configuation script runs a "hello world" binary on the
RISCVMatched prebuilt board found in src/python/gem5/prebuilt/riscvmatched/

Usage
-----

```
scons build/RISCV/gem5.opt
./build/RISCV/gem5.opt \
    configs/example/gem5_library/riscvmatched-hello.py
```
"""

from gem5.isas import ISA
from gem5.prebuilt.riscvmatched.riscvmatched_board import RISCVMatchedBoard
from gem5.resources.resource import obtain_resource, BinaryResource
from gem5.simulate.simulator import Simulator
from gem5.utils.requires import requires

requires(isa_required=ISA.RISCV)

import argparse

parser = argparse.ArgumentParser()
parser.add_argument(
    "--resource",
    required=False,
    type=str,
    default="../test/test_rv",
)
parser.add_argument("--prog_args", required=False, nargs="*", default=[])

args = parser.parse_args()
print("Resource:", args.resource)
print("Program arguments:", args.prog_args)

# instantiate the riscv matched board with default parameters
board = RISCVMatchedBoard()

# set the hello world riscv binary as the board workload
# board.set_se_binary_workload(
#     obtain_resource("riscv-hello", resource_version="1.0.0")
# )
# binary_path = "~/VDB/hnsw/build/bin/hnsw_search_riscv"
# binary_path = "~/VDB/hnsw/test/test_rv"
# binary_path = "~/VDB/hnsw/test/test_rvv"
# binary_path = "~/VDB/hnsw/test/test"
# binary_path = "~/VDB/hnsw/hnsw_search_sifive"
# binary = BinaryResource(binary_path)
# board.set_se_binary_workload(binary)

binary_path = args.resource
binary = BinaryResource(binary_path)
print("Binary Path:", binary_path)
if len(args.prog_args) == 0:
    binary_args = []
else:
    binary_args = args.prog_args[0].split()
print("Program Arguments for Workload:", binary_args)
board.set_se_binary_workload(binary=binary, arguments=binary_args)

# run the simulation with the RISCV Matched board
simulator = Simulator(board=board, full_system=False)
simulator.run()

print(
    "Exiting @ tick {} because {}.".format(
        simulator.get_current_tick(),
        simulator.get_last_exit_event_cause(),
    )
)
