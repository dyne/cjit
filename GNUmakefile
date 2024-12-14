# Copyright (C) 2024 Dyne.org Foundation
#
# This source code is free software; you can redistribute it and/or
# modify it under the terms of the GNU Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This source code is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	Please refer
# to the GNU Public License for more details.
#
# You should have received a copy of the GNU Public License along with
# this source code; if not, , see <https://www.gnu.org/licenses/>.

help:
	@echo "✨ Welcome to the CJIT build system"
	@awk 'BEGIN {FS = ":.*##"; printf "🛟 Usage: make \033[36m<target>\033[0m\n👇 List of targets:\n"} /^[a-zA-Z_0-9-]+:.*?##/ { printf " \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5)} ' GNUmakefile


_: ##
------: ## __ Production targets

musl-linux: ## 🗿 Build a fully static cjit using musl-libc on Linux
	$(MAKE) -f build/musl.mk
	@rm -f .build_done*
	date | tee .build_done_musl

linux-x86: ## 🐧 Build a dynamically linked cjit using libs found on Linux x86
	$(MAKE) -f build/linux.mk
	@rm -f .build_done*
	date | tee .build_done_linux

win-wsl: ## 🪟 Build cjit.exe for WIN64 on an Ubuntu WSL VM using gcc-mingw-w64
	$(MAKE) -f build/win-wsl.mk
	@rm -f .build_done*
	date | tee .build_done_win

win-native: ## 🪟 Build cjit.exe for WIN64 on Windows Server
	cd ./lib/tinycc; bash configure --targetos=WIN32 --config-backtrace=no; make libtcc.a libtcc1.a
	$(MAKE) -f build/win-native.mk
	@rm -f .build_done*
	date | tee .build_done_win

apple-osx: ## 🍎 Build cjit.command for Apple/OSX using clang static
	$(MAKE) -f build/osx.mk
	@rm -f .build_done*
	date | tee .build_done_osx

_: ##
------: ## __ Debugging targets

debug-gdb: ## 🔬 Build using the address sanitizer to detect memory leaks
	$(MAKE) -f build/linux.mk GDB=1
	date | tee .build_done_linux

debug-asan: ## 🔬 Build using the address sanitizer to detect memory leaks
	$(MAKE) -f build/linux.mk ASAN=1
	date | tee .build_done_linux

_: ##
------: ## __ Testing targets

check: ## 🧪 Run all tests using the currently built binary ./cjit
	@./test/bats/bin/bats test/cli.bats
	@./test/bats/bin/bats test/windows.bats
	@if [ -r .build_done_linux ]; then ./test/bats/bin/bats test/dmon.bats; fi


check-ci: ## 🧪 Run all tests using the currently built binary ./cjit
	@./test/bats/bin/bats test/cli.bats
	@./test/bats/bin/bats test/windows.bats
	@./test/bats/bin/bats test/muntar.bats

_: ##
clean: ## 🧹 Clean the source from all built objects
	$(MAKE) -f build/deps.mk clean
	@rm -f cjit cjit.exe cjit.command

PREFIX?=/usr/local
install: cjit
	@install cjit $(PREFIX)/bin
