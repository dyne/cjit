load bats_setup

@test "DMON monitoring of filesystem" {

    # Check if the script is being run by PowerShell
    [ -r ${R}/.build_done_win ] && {
        >&3 echo ">> Skipping DMON test on Windows PowerShell"
        >&3 echo ">> TODO: run in background, test monitor and quit in PS"
        return 0
    }
    ${CJIT} -v 2>&1| grep '^Build: MUSL' && {
        >&3 echo ">> Skipping DMON test on MUSL libc build"
        return 0
    }

    (sleep 1;
     rm -f ${TMP}/dmon_test_create.txt;
     touch ${TMP}/dmon_test_create.txt;
     rm -f ${TMP}/dmon_test_create.txt;
     sleep 2;
     kill -HUP `cat ${TMP}/test_dmon.pid`) &

    run ${CJIT} -p ${TMP}/test_dmon.pid --dmon ${T}/dmon.c -- ${TMP}
    # assert_failure # TODO: cleaner way than kill -HUP
    assert_line --regexp '^CREATE:.*dmon_test_create.txt$'
    assert_line --regexp '^DELETE:.*dmon_test_create.txt$'
}
