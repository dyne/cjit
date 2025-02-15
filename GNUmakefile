# SPDX-FileCopyrightText: 2024 Dyne.org
# SPDX-License-Identifier: GPL-3.0-or-later

# Dear maintainers, welcome to CJIT's build system!
#
# it is all based on GNU Makefile, its core is in build/*.mk
# in particular build/init.mk sets up flags and sources (included top)
# and build/deps.mk sets up dependencies and libs (included bottom)
#
# in case of questions, check the FAQ https://dyne.org/docs/cjit/faq/
# or contact us at https://dyne.org/contact

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

# POSIX system installation paths
PREFIX ?= /usr/local
DATADIR ?= ${PREFIX}/share/cjit
INCDIR ?= ${PREFIX}/include/cjit
MANDIR ?= ${PREFIX}/share/man

help:
	@echo "✨ Welcome to the CJIT build system"
	@awk 'BEGIN {FS = ":.*##"; printf "🛟 Usage: make \033[36m<target>\033[0m\n👇 List of targets:\n"} /^[a-zA-Z_0-9-]+:.*?##/ { printf " \033[36m%-15s\033[0m %s\n", $$1, $$2 } /^##@/ { printf "\n\033[1m%s\033[0m\n", substr($$0, 5)} ' GNUmakefile


_: ##
------: ## __ Production targets

linux: ## 🐧 Build cjit linking shared libs found on Linux (target host arch)
	$(MAKE) -f build/linux.mk embed-posix cjit
	@rm -f .build_done*
	date | tee .build_done_linux

win-wsl: ## 🪟 Build cjit.exe for WIN64 on an Ubuntu WSL VM using gcc-mingw-w64
	$(MAKE) -f build/win-wsl.mk cross-win embed-win cjit.exe
	@rm -f .build_done*
	date | tee .build_done_win

win-native: ## 🪟 Build cjit.exe for WIN64 on Windows Server
	$(MAKE) -f build/win-native.mk embed-win cjit.exe
	@rm -f .build_done*
	date | tee .build_done_win

apple-osx: ## 🍎 Build cjit.command for Apple/OSX using clang static
	$(MAKE) -f build/osx.mk embed-posix cjit.command
	@rm -f .build_done*
	date | tee .build_done_osx

meson:
	rm -rf meson
	meson setup meson build \
		--buildtype release --backend ninja
	ninja -C meson
	cp meson/cjit .

_: ##
------: ## __ Debugging targets

debug-gdb: ## 🔬 Build using the address sanitizer to detect memory leaks
	$(MAKE) -f build/linux.mk embed-posix cjit GDB=1
	date | tee .build_done_linux

debug-asan: ## 🔬 Build using the address sanitizer to detect memory leaks
	$(MAKE) -f build/linux.mk embed-posix cjit ASAN=1
	date | tee .build_done_linux

self-host: ## 💎 Build a CJIT that builts itself (embed its source)
	$(MAKE) -f build/linux.mk embed-posix-source cjit ASAN=1 SELFHOST=1
	date | tee .build_done_linux

_: ##
------: ## __ Testing targets

check: ## 🧪 Run all tests using the currently built binary ./cjit
	@./test/bats/bin/bats test/cli.bats
	@if [ -r .build_done_linux ]; then ./test/bats/bin/bats test/linux.bats; fi
	@./test/bats/bin/bats test/windows.bats
	@./test/bats/bin/bats test/muntar.bats
	@if [ -r .build_done_linux ]; then ./test/bats/bin/bats test/dmon.bats; fi

check-ci: ## 🧪 Run all tests using the currently built binary ./cjit
	@./test/bats/bin/bats test/cli.bats
	@if [ -r .build_done_linux ]; then ./test/bats/bin/bats test/linux.bats; fi
	@./test/bats/bin/bats test/windows.bats
	@./test/bats/bin/bats test/muntar.bats


_: ##
------: ## __ Installation targets

install: ## 🔌 Install the built binaries in PREFIX
	$(info Installing CJIT in ${BUILDDIR}${PREFIX})
	@install -Dm755 cjit ${DESTDIR}${PREFIX}/bin/cjit
	@install -Dm644 docs/cjit.1 ${DESTDIR}${PREFIX}/share/man/man1/cjit.1
	@install -d ${DESTDIR}${DATADIR}
	@cp -ra README.md REUSE.toml LICENSES ${DESTDIR}${DATADIR}/
	@cp -ra examples ${DESTDIR}${DATADIR}/

.PHONY: debian
debian:
	$(info Creating the Debian package)
	@rm -rf debian
	@cp -ra build/debian .
	@cp docs/cjit.1 debian/manpage.1
	@dpkg-buildpackage --no-sign --build=binary


clean: ## 🧹 Clean the source from all built objects
	${MAKE} -C lib/tinycc clean distclean
	${MAKE} -C src clean
	@rm -f cjit cjit.exe cjit.command
	@rm -rf meson
