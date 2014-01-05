/*
 * Copyright (c) 2011, Tom Distler (http://tdistler.com)
 * All rights reserved.
 *
 * The BSD License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * - Neither the name of the tdistler.com nor the names of its contributors may
 *   be used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @page build Building IQA
 * All build artifacts end up in build/&lt;configuration&gt;, where &lt;configuration&gt; is 'debug' or 'release'.
 *
 * @section windows Windows
 * @li Open iqa.sln, select 'Debug' or 'Release', and build. The output is a static library 'iqa.lib'.
 * @li To run the tests under the debugger, first right-click the 'test' project, select Properties -&gt; Configuration Properties -&gt; Debugging and set 'Working Directory' to '$(OutDir)'. Then start the application.
 *
 * @section linux Linux
 * @li Change directories into the root of the IQA branch you want to build.
 * @li Type `make` for a debug build, or `make RELEASE=1` for a release build. The output is a static library 'libiqa.a'.
 * @li Type `make test` (or `make test RELEASE=1`) to build the unit tests.
 * @li Type `make clean` (or `make clean RELEASE=1`) to delete all build artifacts.
 * @li To run the tests, `cd` to the build/&lt;configuration&gt; directory and type `./test`.
 *
 * @code
 * > make clean
 * > make
 * > make test
 * > cd ./build/debug
 * > ./test
 * @endcode
 */
