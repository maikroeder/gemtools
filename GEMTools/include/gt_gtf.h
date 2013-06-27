/*
 * Module to load and manage GTF annotation
 */

#ifndef GT_GTF_H_
#define GT_GTF_H_

#include "gt_commons.h"
#include "gt_string.h"
#include "gt_vector.h"
#include "gt_shash.h"
#include "gt_template.h"

#define GTF_DEFAULT_ENTRIES 10000
#define GTF_MAX_LINE_LENGTH 2048

/*
 * Single gtf entry
 */
typedef struct {
  uint64_t uid;
	uint64_t start; // the start position
	uint64_t end; // the end position
	gt_strand strand; // the strand
	gt_string* type; // the type, i.e. exon, gene
	gt_string* gene_id; // the gene id if it exists
	gt_string* transcript_id; // the transcript id if it exists
	gt_string* gene_type; // the gene id if it exists
} gt_gtf_entry;

typedef struct _gt_gtf_node gt_gtf_node;

struct _gt_gtf_node {
  uint64_t midpoint;
  gt_vector* entries_by_start;
  gt_vector* entries_by_end;
  gt_gtf_node* left;
  gt_gtf_node* right;
} ;

/**
 * Single chromosome reference with
 * all it gtf entries
 */
typedef struct {
	gt_vector* entries; // gt_gtf_entry list
	gt_gtf_node* node; // gt_gtf_entry list
} gt_gtf_ref;

/**
 * GTF file with a map to the chromosome references
 * and all available types (exon, gene, ...).
 */
typedef struct {
	gt_shash* refs; // maps from the ref name to the ref char* -> gt_gtf_ref*
	gt_shash* types; // maps from the type name to the gt_string type ref char* -> gt_string*
	gt_shash* gene_ids; // maps from char* to gt_string* for gene_ids char* -> gt_string*
	gt_shash* transcript_ids; // maps from char* to gt_string* for gene_ids char* -> gt_string*
	gt_shash* gene_types; // maps from char* to gt_string* for gene_types char* -> gt_string*
}gt_gtf;

/**
 * gtf hit that are filled by the template search methods
 */
typedef struct {
  gt_vector* exon_hits; // contains an entry for each map or map pair with the hit target id (gene_id)
}gt_gtf_hits;


typedef struct {
  gt_map* map;
  gt_map** mmap;
  gt_mmap_attributes* map_attributes;
  gt_shash* transcripts;
  float_t exon_overlap;
  float_t junction_hits;
  uint64_t num_junctions;
  uint64_t intron_length;
  uint64_t num_template_blocks;
  bool is_protein_coding;
  bool pairs_transcript;
  bool pairs_splits;
}gt_gtf_hit;

GT_INLINE gt_gtf_hit* gt_gtf_hit_new(void);
GT_INLINE void gt_gtf_hit_delete(gt_gtf_hit* hit);

/**
 * Create and delete gtf
 */
GT_INLINE gt_gtf* gt_gtf_new(void);
GT_INLINE void gt_gtf_delete(gt_gtf* const gtf);

/**
 * Create and delete hits
 */
GT_INLINE gt_gtf_hits* gt_gtf_hits_new(void);
GT_INLINE void gt_gtf_hits_delete(gt_gtf_hits* const hits);
GT_INLINE void gt_gtf_hits_clear(gt_gtf_hits* const hits);

/**
 * Create and delete new gtf_entries
 */
GT_INLINE gt_gtf_entry* gt_gtf_entry_new(const uint64_t start, const uint64_t end, const gt_strand strand, gt_string* const type);
GT_INLINE void gt_gtf_entry_delete(gt_gtf_entry* const entry);

/**
 * Create and delete new chromosome refs
 */
GT_INLINE gt_gtf_ref* gt_gtf_ref_new(void);
GT_INLINE void gt_gtf_ref_delete(gt_gtf_ref* const ref);

/**
 * Parse GTF files and return a new gt_gtf*. The ref entries
 * will be sorted by star,end,type
 */
GT_INLINE gt_gtf* gt_gtf_read(FILE* input);

/**
 * Access the chromosome refs
 */
GT_INLINE gt_gtf_ref* gt_gtf_get_ref(const gt_gtf* const gtf, char* const name);
GT_INLINE bool gt_gtf_contains_ref(const gt_gtf* const gtf, char* const name);

/**
 * Access available types
 */
GT_INLINE gt_string* gt_gtf_get_type(const gt_gtf* const gtf, char* const type);
GT_INLINE bool gt_gtf_contains_type(const gt_gtf* const gtf, char* const name);

/**
 * Access available gene_ids
 */
GT_INLINE gt_string* gt_gtf_get_gene_id(const gt_gtf* const gtf, char* const name);
GT_INLINE bool gt_gtf_contains_gene_id(const gt_gtf* const gtf, char* const name);

/**
 * Access available gene_ids
 */
GT_INLINE gt_string* gt_gtf_get_transcript_id(const gt_gtf* const gtf, char* const name);
GT_INLINE bool gt_gtf_contains_transcript_id(const gt_gtf* const gtf, char* const name);

/**
 * Access available gene_types
 */
GT_INLINE gt_string* gt_gtf_get_gene_type(const gt_gtf* const gtf, char* const name);
GT_INLINE bool gt_gtf_contains_gene_type(const gt_gtf* const gtf, char* const name);

/**
 * Search
 */
/**
 * Search for annotation that overlap with the specified region. The matching entries will be added to the target
 * vector. Note that the target vector is cleared at the beginning of the method!
 */
GT_INLINE void gt_gtf_search(const gt_gtf* const gtf, gt_vector* const target, char* const ref, const uint64_t start, const uint64_t end);
/**
 * Search for exons that overlap with the given template mappings.
 */
GT_INLINE void gt_gtf_search_template_for_exons(const gt_gtf* const gtf, gt_gtf_hits* const hits, gt_template* const template_src);

#endif /* GT_GTF_H_ */
