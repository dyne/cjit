load bats_setup

@test "DMON monitoring of filesystem" {

    ${CJIT} -v 2>&1| grep '^Build: MUSL' && {
        >&3 echo ">> Skipping DMON test on MUSL libc build"
        return 0
    }

	(
	 deadline=$((SECONDS + 10))
	 while [ ! -s "${TMP}/test_dmon.pid" ] && [ "$SECONDS" -lt "$deadline" ]; do sleep 0.05; done
	 [ -s "${TMP}/test_dmon.pid" ] || exit 1
     rm -f ${TMP}/dmon_test_create.txt;
     touch ${TMP}/dmon_test_create.txt;
     rm -f ${TMP}/dmon_test_create.txt;
	) &

	# The helper exits after the observed event subset; no host-specific signal.
    run ${CJIT} -p ${TMP}/test_dmon.pid -I${T} ${T}/dmon.c -- --count=2 ${TMP}
    assert_line --regexp '^CREATE:.*dmon_test_create.txt$'
    assert_line --regexp '^DELETE:.*dmon_test_create.txt$'
}
