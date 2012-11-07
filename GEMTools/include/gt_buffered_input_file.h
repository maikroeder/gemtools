/*
 * PROJECT: GEM-Tools library
 * FILE: gt_buffered_input_file.h
 * DATE: 01/06/2012
 * DESCRIPTION: // TODO
 */

#ifndef GT_BUFFERED_INPUT_FILE_H_
#define GT_BUFFERED_INPUT_FILE_H_

#include "gt_commons.h"
#include "gt_input_file.h"
#include "gt_template.h"
#include "gt_buffered_output_file.h"

// Codes gt_status
#define GT_BMI_OK 1
#define GT_BMI_FAIL -1
#define GT_BMI_EOF 0
// PE (Parsing Errors)
// TODO


typedef struct {
  /* Input file */
  gt_input_file* input_file;
  /* Block buffer and cursors */
  uint64_t block_id;
  gt_vector* block_buffer;
  char* cursor;
  uint64_t lines_in_buffer;
  uint64_t current_line_num;
} gt_buffered_input_file;

/*
 * Checkers
 */
#define GT_BUFFERED_INPUT_FILE_CHECK(buffered_input_file) gt_fatal_check( \
  buffered_input_file==NULL||buffered_input_file->input_file==NULL|| \
  buffered_input_file->block_buffer==NULL||buffered_input_file->cursor==NULL,NULL_HANDLER)

/*
 * Buffered Input File Handlers
 */
gt_buffered_input_file* gt_buffered_input_file_new(gt_input_file* const input_file);
gt_status gt_buffered_input_file_close(gt_buffered_input_file* const buffered_input_file);
GT_INLINE uint64_t gt_buffered_input_file_get_cursor_pos(gt_buffered_input_file* const buffered_input_file);
GT_INLINE bool gt_buffered_input_file_eob(gt_buffered_input_file* const buffered_input_file);
GT_INLINE gt_status gt_buffered_input_file_get_block(
    gt_buffered_input_file* const buffered_input_file,const uint64_t num_lines,const bool use_mutex);
GT_INLINE gt_status gt_buffered_input_file_add_lines_to_block(
    gt_buffered_input_file* const buffered_input_file,const uint64_t num_lines);

/*
 * Processing Macros (for parsing)
 */
#define GT_INPUT_FILE_SKIP_LINE(buffered_map_input) { \
  while (buffered_map_input->cursor[0]!=EOL) { \
    ++buffered_map_input->cursor; \
  } \
  buffered_map_input->cursor[0]=EOS; \
  ++buffered_map_input->cursor; \
  if (gt_expect_false(!gt_buffered_input_file_eob(buffered_map_input) && buffered_map_input->cursor[0]==DOS_EOL)) { \
    ++buffered_map_input->cursor; \
  } \
  ++buffered_map_input->current_line_num; \
}

#endif /* GT_BUFFERED_INPUT_FILE_H_ */