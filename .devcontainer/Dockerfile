FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-24.04

ARG REINSTALL_CMAKE_VERSION_FROM_SOURCE="3.22.2"

# Optionally install the cmake for vcpkg
COPY ./reinstall-cmake.sh /tmp/

RUN if [ "${REINSTALL_CMAKE_VERSION_FROM_SOURCE}" != "none" ]; then \
        chmod +x /tmp/reinstall-cmake.sh && /tmp/reinstall-cmake.sh ${REINSTALL_CMAKE_VERSION_FROM_SOURCE}; \
    fi \
    && rm -f /tmp/reinstall-cmake.sh

# [Optional] Uncomment this section to install additional vcpkg ports.
# RUN su vscode -c "${VCPKG_ROOT}/vcpkg install <your-port-name-here>"

# [Optional] Uncomment this section to install additional packages.
# RUN apt-get update && export DEBIAN_FRONTEND=noninteractive \
#     && apt-get -y install --no-install-recommends <your-package-list-here>

# Clone dependency repos

# nlohmann/json
RUN git clone https://github.com/nlohmann/json.git
# uriparser
RUN git clone https://github.com/uriparser/uriparser

# Install dependencies
# Install in parts for better caching
RUN apt-get update && apt-get install -y \
git \
cmake

RUN apt-get install -y \
g++ \
libboost-all-dev


RUN apt-get install -y lcov

RUN apt-get install -y clang-format

# Build nlohmann/json
WORKDIR /json
RUN mkdir build
WORKDIR /json/build
RUN cmake ..
RUN make
RUN make install

WORKDIR /

# Build uriparser
WORKDIR /uriparser
RUN mkdir build
WORKDIR /uriparser/build
RUN cmake -DCMAKE_BUILD_TYPE=Release -DURIPARSER_BUILD_WCHAR_T=OFF -DURIPARSER_BUILD_TESTS=OFF -DURIPARSER_BUILD_DOCS=OFF ..
RUN make
RUN make install
