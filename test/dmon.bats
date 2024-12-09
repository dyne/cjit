load bats_setup

@test "DMON monitoring of filesystem" {
    (sleep .5;
     rm -f ${TMP}/dmon_test_create.txt;
     touch ${TMP}/dmon_test_create.txt;
     rm -f ${TMP}/dmon_test_create.txt;
     sleep 2;
     kill -HUP `cat ${TMP}/test_dmon.pid`) &

    run ${CJIT} -p ${TMP}/test_dmon.pid --dmon ${T}/dmon.c -- ${TMP}
    assert_failure # TODO: cleaner way than kill -HUP
    assert_line --regexp '^CREATE:.*dmon_test_create.txt$'
    assert_line --regexp '^DELETE:.*dmon_test_create.txt$'
}
