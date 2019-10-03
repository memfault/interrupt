# Embedded unit tests!

# Setup

- Linux - `sudo apt-get install cpputest lcov`
- OSX - `brew install cpputest && brew install lcov`

# Running tests

`inv fw.tests`

# Directory structure

```
├── Makefile // Invokes all the unit tests
├── MakefileWorker.mk // Comes from CppUTest itself
├── MakefileWorkerOverrides.mk // memfault injected overrides
├── build
│   [...] // Where all the tests wind up
├── fakes
│   // fakes for unit tests
├── fakes
│   // mocks for unit tests
├── makefiles // Each c file you unit test has a makefile here
│   └── Makefile_<module_name>.mk
|   [...]
└── src // test source files
└── test_memfault_storage.cpp
```

# Adding a test

- Add a new test makefile under test/makefiles/. These just list the sources you
  will compile
- Add a new test file under tests/src for the module you want to test
- `inv fw.test`
