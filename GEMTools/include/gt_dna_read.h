/*
 * PROJECT: GEM-Tools library
 * FILE: gt_dna_read.h
 * DATE: 20/08/2012
 * AUTHOR(S): Santiago Marco-Sola <santiagomsola@gmail.com>
 * DESCRIPTION: Simple data structure to store genomic reads
 */

#ifndef GT_DNA_READ_H_
#define GT_DNA_READ_H_

#include "gt_commons.h"

typedef struct {
  gt_string* tag;
  gt_string* read;
  gt_string* qualities;
  gt_shash* attributes;
} gt_dna_read;

typedef enum { GT_QUALS_OFFSET_33, GT_QUALS_OFFSET_64 } gt_qualities_offset_t;

/*
 * Checkers
 */
#define GT_DNA_READ_CHECK(the_read) \
  GT_NULL_CHECK(the_read); \
  GT_STRING_CHECK(the_read->tag); \
  GT_STRING_CHECK(the_read->read); \
  GT_STRING_CHECK(the_read->qualities)

#define gt_is_valid_quality(character) (33 <= (character))

/*
 * Constructor
 */
GT_INLINE gt_dna_read* gt_dna_read_new(void);
GT_INLINE void gt_dna_read_clear(gt_dna_read* read);
GT_INLINE void gt_dna_read_delete(gt_dna_read* read);

/*
 * Accessors
 */
GT_INLINE void gt_dna_read_set_ntag(gt_dna_read* read,char* const text,const uint64_t length);
GT_INLINE void gt_dna_read_set_tag(gt_dna_read* read,char* const text);
GT_INLINE char* gt_dna_read_get_tag(gt_dna_read* const read);

GT_INLINE void gt_dna_read_set_nread(gt_dna_read* read,char* const text,const uint64_t length);
GT_INLINE void gt_dna_read_set_read(gt_dna_read* read,char* const text);
GT_INLINE char* gt_dna_read_get_read(gt_dna_read* const read);

GT_INLINE void gt_dna_read_set_nqualities(gt_dna_read* read,char* const text,const uint64_t length);
GT_INLINE void gt_dna_read_set_qualities(gt_dna_read* read,char* const text);
GT_INLINE char* gt_dna_read_get_qualities(gt_dna_read* const read);

/*
 * Attribute accessors
 */
#define GT_ATTR_QUALITY_OFFSET "QUAL_OFFSET"

/*
 * Handlers
 */
GT_INLINE gt_status gt_dna_read_deduce_qualities_offset(gt_dna_read* const read,gt_qualities_offset_t* qualities_offset_type);

GT_INLINE uint64_t gt_dna_read_quality_trim(gt_dna_read* const read,const uint8_t quality_threshold,const uint64_t min_length); // TODO
GT_INLINE uint64_t gt_dna_read_hard_trim(gt_dna_read* const read,const uint64_t length);

#endif /* GT_DNA_READ_H_ */
