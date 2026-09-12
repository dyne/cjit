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
	$(MAKE) -f build/win-wsl.mk cross-win embed-win cjit.exe cjit-ar.exe
	@rm -f .build_done*
	date | tee .build_done_win

win-mingw: ## 🪟 Build cjit.exe for WIN64 on Windows using MinGW
	"$(MAKE)" -f build/win-mingw.mk embed-win cjit.exe cjit-ar.exe
	@rm -f .build_done*
	date | tee .build_done_win

win-msvc: ## 🪟 Build cjit.exe for WIN64 on Windows using MSVC
	"$(MAKE)" -f build/win-msvc.mk win-msvc

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
	@find src lib/muntarfs -type f \( -name '*.o' -o -name '*.d' -o -name '*.gcda' -o -name '*.gcno' \) -delete
	@$(MAKE) -C lib/tinycc clean distclean
	$(MAKE) -f build/linux.mk embed-posix cjit ASAN=1
	date | tee .build_done_linux

self-host: ## 💎 Build a CJIT that builts itself (embed its source)
	$(MAKE) -f build/linux.mk embed-posix-source cjit ASAN=1 SELFHOST=1
	date | tee .build_done_linux

_: ##
------: ## __ Testing targets

check: ## 🧪 Run all tests using the currently built binary ./cjit
	@$(MAKE) check-unit
	@./test/bats/bin/bats test/cli.bats
	@if [ -r .build_done_linux ]; then ./test/bats/bin/bats test/linux.bats; fi
	@./test/bats/bin/bats test/windows.bats
	@./test/bats/bin/bats test/muntar.bats
	@if [ -r .build_done_linux ]; then ./test/bats/bin/bats test/dmon.bats; fi

check-ci: ## 🧪 Run all tests using the currently built binary ./cjit
	@$(MAKE) check-unit
	@./test/bats/bin/bats test/cli.bats
	@if [ -r .build_done_linux ]; then ./test/bats/bin/bats test/linux.bats; fi
	@./test/bats/bin/bats test/windows.bats
	@./test/bats/bin/bats test/muntar.bats

COVERAGE_FLAGS := --coverage -O0 -g
COVERAGE_SOURCES := src/file.c src/cjit.c src/cjit-ar.c src/main.c src/support/source_files.c \
	src/support/string_list.c src/array.c src/app/execute_source.c \
	src/app/compile_object.c src/app/build_executable.c src/app/print_status.c \
	src/app/extract_assets.c src/app/extract_archive.c src/adapters/cli/route_parser.c \
	src/adapters/cli/render_response.c src/adapters/compiler/tinycc_adapter.c \
	src/adapters/fs/local_filesystem.c src/adapters/fs/local_asset.c \
	src/adapters/platform/library_resolver_posix.c \
	src/adapters/platform/library_resolver_windows.c src/adapters/platform/runtime_platform.c \
	lib/muntarfs/muntarfs_runtime.c lib/muntarfs/muntar.c lib/muntarfs/tinflate.c \
	lib/muntarfs/tinfgzip.c

ifeq ($(findstring clang,$(shell $(CC) --version 2>/dev/null | head -1)),clang)
COVERAGE_TOOL ?= llvm-cov gcov
else
COVERAGE_TOOL ?= gcov
endif

coverage: ## 📊 Build, test, and summarize maintained-source coverage (no threshold)
	@$(MAKE) coverage-clean
	@$(MAKE) clean
	@$(MAKE) linux
	@find src lib/muntarfs -type f \( -name '*.o' -o -name '*.d' \) -delete
	@$(MAKE) -f build/linux.mk embed-posix cjit CFLAGS="$(COVERAGE_FLAGS)" LDFLAGS="$(COVERAGE_FLAGS)"
	@$(MAKE) check-ci
	@$(MAKE) coverage-report
	@find src lib/muntarfs -type f \( -name '*.o' -o -name '*.d' \) -delete
	@$(MAKE) coverage-clean

coverage-report: ## 📊 Print line coverage for maintained sources only
	@for source_file in $(COVERAGE_SOURCES); do \
		coverage_output="$$($(COVERAGE_TOOL) -n "$$source_file")" || exit $$?; \
		printf '%s\n' "$$coverage_output" | awk -v source="File '$$source_file'" \
			'$$0 == source { show = 1 } /^File / && $$0 != source { show = 0 } show && (/^File / || /^Lines executed:/) { print; if (/^Lines executed:/) show = 0 }'; \
	done

coverage-clean: ## 🧹 Remove compiler-native coverage profiles
	@find src lib/muntarfs lib/tinycc -type f \( -name '*.gcda' -o -name '*.gcno' \) -delete

UNIT_BINS := test/source_files_unit.bin test/source_files_edge_unit.bin \
	test/cli_parser_unit.bin test/cli_route_unit.bin test/cli_render_unit.bin \
	test/app_slices_unit.bin test/string_list_unit.bin test/file_unit.bin \
	test/runtime_cache_unit.bin test/cjit_lifecycle_unit.bin

test/source_files_unit.bin: UNIT_SOURCES := src/support/source_files.c src/support/cwalk.c
test/source_files_edge_unit.bin: UNIT_SOURCES := src/support/source_files.c src/support/cwalk.c
test/cli_parser_unit.bin: UNIT_SOURCES := src/adapters/cli/route_parser.c
test/cli_route_unit.bin: UNIT_SOURCES := src/adapters/cli/route_parser.c
test/cli_render_unit.bin: UNIT_SOURCES := src/adapters/cli/render_response.c
test/app_slices_unit.bin: UNIT_SOURCES := src/app/execute_source.c src/app/compile_object.c src/app/build_executable.c src/app/print_status.c src/app/extract_assets.c src/app/extract_archive.c
test/string_list_unit.bin: UNIT_SOURCES := src/support/string_list.c src/array.c
test/file_unit.bin: UNIT_SOURCES := src/file.c src/support/cwalk.c
test/runtime_cache_unit.bin: UNIT_SOURCES := src/adapters/fs/local_filesystem.c src/support/cwalk.c
test/runtime_cache_unit.bin: CFLAGS += -DVERSION=\"test-runtime\"
test/cjit_lifecycle_unit.bin: UNIT_SOURCES := src/cjit.c src/support/string_list.c src/array.c src/support/source_files.c src/support/cwalk.c
test/cjit_lifecycle_unit.bin: CFLAGS += -DSHAREDTCC -DVERSION=\"unit\" -Ilib/tinycc

$(UNIT_BINS): test/%_unit.bin: test/%_unit.c $(UNIT_SOURCES)
	$(CC) $(CFLAGS) -Isrc -o $@ $< $(UNIT_SOURCES)

check-unit: $(UNIT_BINS) ## 🧪 Run small direct C tests for pure support logic
	@for test_binary in $(UNIT_BINS); do \
		printf 'UNIT %s\n' "$$test_binary"; \
		"./$$test_binary" || exit $$?; \
	done


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
	"${MAKE}" -C lib/tinycc clean distclean
	"${MAKE}" -C src clean
	@find src lib/muntarfs -type f \( -name '*.o' -o -name '*.d' \) -delete
	@rm -f cjit cjit.exe cjit-ar.exe cjit.command libtcc.dll
	@rm -f $(UNIT_BINS) test/source_files_unit
	@rm -rf meson
