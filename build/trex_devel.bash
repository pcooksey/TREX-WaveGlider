#!/bin/bash
#####################################################################
# Software License Agreement (BSD License)
# 
#  Copyright (c) 2011, MBARI.
#  All rights reserved.
# 
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
# 
#   * Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#   * Redistributions in binary form must reproduce the above
#     copyright notice, this list of conditions and the following
#     disclaimer in the documentation and/or other materials provided
#     with the distribution.
#   * Neither the name of the TREX Project nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
# 
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
#  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
#  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
#  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
#  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.
#####################################################################


TREX_LIBS=/home/pcooksey/TREX-WaveGlider/build/trex/utils:/home/pcooksey/TREX-WaveGlider/build/trex/domain:/home/pcooksey/TREX-WaveGlider/build/trex/transaction:/home/pcooksey/TREX-WaveGlider/build/trex/agent:/home/pcooksey/TREX-WaveGlider/build/extra/europa/core:/home/pcooksey/TREX-WaveGlider/build/extra/europa:/home/pcooksey/TREX-WaveGlider/build/extra/examples/lightswitch:/home/pcooksey/TREX-WaveGlider/build/extra/examples/rover:/home/pcooksey/TREX-WaveGlider/build/extra/examples/shopping:/home/pcooksey/TREX-WaveGlider/build/extra/examples/simplerobot:/usr/local/lib
TREX_BINS=/home/pcooksey/TREX-WaveGlider/build/trex:/home/pcooksey/TREX-WaveGlider/trex-0.5.1/scripts

EUROPA_HOME=/home/pcooksey/europa

if [ -d "${EUROPA_HOME}" ]; then 
    export EUROPA_HOME
    TREX_LIBS=${TREX_LIBS}:/home/pcooksey/europa/lib
fi

platform='unknown'
lib_path_var=LD_LIBRARY_PATH
unamestr=`uname`
if [ "$unamestr" = 'Darwin' ]; then
    if [ -n "${DYLD_FALLBACK_LIBRARY_PATH+x}" ]; then
	export DYLD_FALLBACK_LIBRARY_PATH=${TREX_LIBS}:${DYLD_FALLBACK_LIBRARY_PATH}
    else
	export DYLD_FALLBACK_LIBRARY_PATH=${TREX_LIBS}
    fi
    export DYLD_BIND_AT_LAUNCH=YES
else 
    if [ -n "${LD_LIBRARY_PATH+x}" ]; then
	export LD_LIBRARY_PATH=${TREX_LIBS}:${LD_LIBRARY_PATH}
    else
	export LD_LIBRARY_PATH=${TREX_LIBS}
    fi
fi
   

if [ -n "${PATH+x}" ]; then
    export PATH=${TREX_BINS}:${PATH}
else 
    export PATH=${TREX_BINS}
fi

export TREX_PATH=/home/pcooksey/TREX-WaveGlider/trex-0.5.1/cfg:/home/pcooksey/TREX-WaveGlider/build/extra/europa/cfg:/home/pcooksey/TREX-WaveGlider/trex-0.5.1/extra/europa/cfg:/home/pcooksey/TREX-WaveGlider/build/extra/europa:/home/pcooksey/TREX-WaveGlider/trex-0.5.1/extra/examples/lightswitch/cfg:/home/pcooksey/TREX-WaveGlider/build/extra/examples/lightswitch:/home/pcooksey/TREX-WaveGlider/trex-0.5.1/extra/examples/rover/cfg:/home/pcooksey/TREX-WaveGlider/build/extra/examples/rover:/home/pcooksey/TREX-WaveGlider/trex-0.5.1/extra/examples/shopping/cfg:/home/pcooksey/TREX-WaveGlider/build/extra/examples/shopping:/home/pcooksey/TREX-WaveGlider/trex-0.5.1/extra/examples/simplerobot/cfg:/home/pcooksey/TREX-WaveGlider/build/extra/examples/simplerobot

export TREX_LOG_DIR=/home/pcooksey/TREX-WaveGlider/build/log

# Python support 
py_libs=

if [ -n "${py_libs}" ]; then
    if [ -n "${PYTHONPATH+x}" ]; then 
	export PYTHONPATH=${py_libs}:${PYTHONPATH}
    else
	export PYTHONPATH=${py_libs}
    fi
fi

