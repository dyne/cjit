__LINE__
# 10
__LINE__
# line 20
__LINE__
# 64mb
__LINE__
# line 30
__LINE__
#define LINE1 40
# line LINE1
__LINE__ __FILE__
#define LINE2 50
# line LINE2 "file1"
__LINE__ __FILE__
#define LINE3 60
#define FILE "file2"
# line LINE3 FILE
__LINE__ __FILE__
