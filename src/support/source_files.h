#ifndef CJIT_SUPPORT_SOURCE_FILES_H
#define CJIT_SUPPORT_SOURCE_FILES_H

/**
 * Classify a path according to CJIT's current source-file rules.
 *
 * Returns:
 *   1 for C-family source files
 *   0 for paths without an extension
 *  -1 for non-source files with an extension
 */
int cjit_classify_source_path(const char *path);

#endif
