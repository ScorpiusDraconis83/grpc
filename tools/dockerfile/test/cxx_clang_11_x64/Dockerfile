# Copyright 2021 gRPC authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

FROM silkeh/clang:11-bullseye

RUN apt update && apt -y upgrade && apt install -y build-essential curl git time wget zip && apt clean
#=================
# Setup git to access working directory across docker boundary.
# This avoids the "fatal: detected dubious ownership in repository XYZ"
# git error.

RUN git config --global --add safe.directory '*'
RUN git config --global protocol.file.allow always

#====================
# run_tests.py python dependencies

# Basic python dependencies to be able to run tools/run_tests python scripts
# These dependencies are not sufficient to build gRPC Python, gRPC Python
# deps are defined elsewhere (e.g. python_deps.include)
RUN apt-get update && apt-get install -y \
  python3 \
  python3-pip \
  python3-setuptools \
  python3-yaml \
  && apt-get clean

# use pinned version of pip to avoid sudden breakages
RUN python3 -m pip install --upgrade pip==19.3.1

# TODO(jtattermusch): currently six is needed for tools/run_tests scripts
# but since our python2 usage is deprecated, we should get rid of it.
RUN python3 -m pip install six==1.16.0

# Google Cloud Platform API libraries
# These are needed for uploading test results to BigQuery (e.g. by tools/run_tests scripts)
RUN python3 -m pip install --upgrade google-auth==1.23.0 google-api-python-client==1.12.8 oauth2client==4.1.0


# Some cxx tests depend on the twisted package
RUN python3 -m pip install twisted

#=================
# Install cmake using the install script.
RUN curl -sSL -o cmake-linux.sh https://github.com/Kitware/CMake/releases/download/v3.16.1/cmake-3.16.1-Linux-x86_64.sh \
    && sh cmake-linux.sh -- --skip-license --prefix=/usr \
    && rm cmake-linux.sh

#=================
# Install ccache

# Install ccache from source since ccache 3.x packaged with most linux distributions
# does not support Redis backend for caching.
RUN curl -sSL -o ccache.tar.gz https://github.com/ccache/ccache/releases/download/v4.7.5/ccache-4.7.5.tar.gz \
    && tar -zxf ccache.tar.gz \
    && cd ccache-4.7.5 \
    && mkdir build && cd build \
    && cmake -DCMAKE_BUILD_TYPE=Release -DZSTD_FROM_INTERNET=ON -DHIREDIS_FROM_INTERNET=ON .. \
    && make -j4 && make install \
    && cd ../.. \
    && rm -rf ccache-4.7.5 ccache.tar.gz


RUN mkdir /var/local/jenkins


# Define the default command.
CMD ["bash"]
