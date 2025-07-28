# SST Tools

Debugging is both an art and a discipline. There are a host of debug features provided in `sst-core`. In addition, there are an unbounded number of user developed features realized in their individual workflows and SST components. The developer may be overwhelmed with the debug learning curve and resort to a printf based debug methodology only then to have to support a set of post-processing scripts to deal with an overwhelming amount of data followed by a never-ending cycle of recompiling and rerunning code to try to root cause a problem.

Improving awareness, sharing best known methods, and speeding up the learning curve can have significant ROI for teams wrestling with large SST simulations. This can be followed by a virtuous cycle of improved debug support and components designed to leverage them.

The documentation and code samples provided here are intended provide a "point of entry" to help developers quickly get started and navigate the best known methods, features, and use cases to best meet their needs. This repository also serves as an experimental protyping sandbox for future debug capabilities.

## Document List

- [SST Tools Users Guide](documentation/sst-tools-users-guide.md)

    Lists of features, summary information,  and where to find supporting documentation and samples.

- [SST Debug Handbook](documentation/sst-v15.0-debug-handbook.md)
  
    Walks through the latest debugging support in SST along with simple usage examples and how to deal with known issues.

## Building and Running Sample Code
Design and debug utilities, examples, and methods for SST developers

    git clone git@github.com:tactcomplabs/sst-tools.git
    cd build
    cmake -DSST_TOOLS_ENABLE_TESTING=ON ..
    make && make install
    ctest

CMake configuration options include:

    SST_TOOLS_MPI            - Enable MPI for testing if available (OFF)
    SST_TOOLS_SOCKETS        - Enable tests, like CLIDBG, that require sockets (ON)
    SST_TOOLS_CLEAN_TESTS    - delete saved checkpoint directories (ON)
    SST_TOOLS_ENABLE_TESTING - enable all tests (OFF)
    SST_TOOLS_WERROR         - enabled -Werror compile flag (ON)

## Experimental Code

### Component Debug Probe Evaluation Model
For information on instrumenting code using the component debug probe methodology see [dbgcli documentation](test/dbgcli/README.md)

### Checkpoint API Prototype
For information on generating json schema files for checkpoints and interacting with them using a Python API see the [cptapi documentation](test/cptapi/README.md).

## Contributing

We welcome outside contributions from corporate, academic and individual
developers. However, there are a number of fundamental ground rules that you
must adhere to in order to participate. These rules are outlined as follows:

* By contributing to this code, one must agree to the licensing described in
the top-level [LICENSE](LICENSE) file.
* All code must adhere to the existing C++ coding style. While we are somewhat
flexible in basic style, you will adhere to what is currently in place. This
includes camel case C++ methods and inline comments. Uncommented, complicated
algorithmic constructs will be rejected.
* We support compilaton and adherence to C++ standard methods. All new methods
and variables contained within public, private and protected class methods must
be commented using the existing Doxygen-style formatting. All new classes must
also include Doxygen blocks in the new header files. Any pull requests that
lack these features will be rejected.
* All changes to functionality and the API infrastructure must be accompanied
by complementary tests All external pull requests **must** target the `devel`
branch. No external pull requests will be accepted to the master branch.
* All external pull requests must contain sufficient documentation in the pull
request comments in order to be accepted.

## Integration with SST
All code in this file is compiled with the -Werror flag. The only warnings 
that may be disabled are when including the external SST code. The following
methodology is used for SST integration.

* You should only `#include "SST.h"` and none of the '#include "sst/core/..." in
the `sst-tools` source files.
* All warnings which need to be disabled just for SST source code outside of this
repo can be disabled in `SST.h` using `#pragma`

## License

See the [LICENSE](./LICENSE) file

## Authors
* John Leidel
* Ken Griesser
* Shannon Kuntz

