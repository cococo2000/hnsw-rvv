# Copyright (c) 2024 Barcelona Supercomputing Center
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors
# may be used to endorse or promote products derived from this software without
# specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

"""
This script demonstrates how to run RISC-V vector-enabled binaries in SE mode
with gem5. It accepts the number of CORES, VLEN, and ELEN as optional
parameters, as well as the resource name to run. If no resource name is
provided, a list of available resources will be displayed. If one is given the
simulation will then execute the specified resource binary with the selected
parameters until completion.


Usage
-----

# Compile gem5 for RISC-V
scons build/RISCV/gem5.opt

# Run the simulation
./build/RISCV/gem5.opt configs/example/gem5_library/riscv-rvv-example.py \
    [-c CORES] [-v VLEN] [-e ELEN] <resource>

"""

import argparse

# from m5.objects import RiscvO3CPU, RiscvMinorCPU, AddrRange
# from m5.objects.BaseMinorCPU import *

from gem5.components.boards.simple_board import SimpleBoard
# from gem5.components.cachehierarchies.classic.private_l1_private_l2_cache_hierarchy import (
#     PrivateL1PrivateL2CacheHierarchy,
# )
# from gem5.components.memory import (
#     SingleChannelDDR3_1600,
#     SingleChannelDDR4_2400,
# )
# from gem5.components.processors.base_cpu_core import BaseCPUCore
from gem5.components.processors.base_cpu_processor import BaseCPUProcessor
from gem5.isas import ISA
from gem5.resources.resource import obtain_resource, BinaryResource
from gem5.simulate.simulator import Simulator
from gem5.utils.requires import requires


from riscvmatched.riscvmatched_board import RISCVMatchedBoard

requires(isa_required=ISA.RISCV)


parser = argparse.ArgumentParser()
parser.add_argument(
    "--resource",
    required=False,
    type=str,
    # default="../build/bin/hnsw_search_riscv",
    default="../test/test_rv",
)
# parser.add_argument("-c", "--cores", required=False, type=int, default=1)
parser.add_argument("--clk_freq", required=False, type=str, default="1.2GHz")
parser.add_argument("--l1i_size", required=False, type=str, default="32KiB")
parser.add_argument("--l1i_assoc", required=False, type=int, default=4)
parser.add_argument("--l1d_size", required=False, type=str, default="32KiB")
parser.add_argument("--l1d_assoc", required=False, type=int, default=8)
parser.add_argument("--l2size", required=False, type=str, default="2MiB")
parser.add_argument("--l2assoc", required=False, type=int, default=16)
parser.add_argument("-v", "--vlen", required=False, type=int, default=256)
parser.add_argument("-e", "--elen", required=False, type=int, default=64)
parser.add_argument("--prog_args", required=False, nargs='*', default=[])

args = parser.parse_args()

print("Resource:", args.resource)
# print("Cores:", args.cores)
print("Clock Frequency:", args.clk_freq)
print("L1i Size:", args.l1i_size)
print("L1i Assoc:", args.l1i_assoc)
print("L1d Size:", args.l1d_size)
print("L1d Assoc:", args.l1d_assoc)
print("L2 Size:", args.l2size)
print("L2 Assoc:", args.l2assoc)
print("VLEN:", args.vlen)
print("ELEN:", args.elen)
print("Program Arguments:", args.prog_args)

board = RISCVMatchedBoard(
    clk_freq=args.clk_freq,
    l1i_size=args.l1i_size,
    l1i_assoc=args.l1i_assoc,
    l1d_size=args.l1d_size,
    l1d_assoc=args.l1d_assoc,
    l2_size=args.l2size,
    l2_assoc=args.l2assoc,
    vlen=args.vlen,
    elen=args.elen,
)
# print("Board:", board)

# binary = obtain_resource(args.resource)
# binary_path = "../test/test_rvv"
# binary_path = "../build/bin/hnsw_search_riscv_rvv"


binary_path = args.resource
binary = BinaryResource(binary_path)
print("Binary Path:", binary_path)
if len(args.prog_args) == 0:
    binary_args = []
else:
    binary_args = args.prog_args[0].split()
print("Program Arguments for Workload:", binary_args)
board.set_se_binary_workload(binary=binary, arguments=binary_args)


simulator = Simulator(board=board, full_system=False)
print("Beginning simulation!")
simulator.run()
