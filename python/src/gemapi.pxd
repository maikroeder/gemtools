# cdef extern from "stdbool.h":
#     ctypedef int bool
from libcpp cimport bool
from libc.stdlib cimport malloc, free

cdef extern from "stdio.h":
  printf(char* string)

cdef extern from "stdlib.h":
    int strcmp(char *a, char *b)

cdef extern from "stdint.h":
    ctypedef int uint64_t
    ctypedef int int64_t
    ctypedef int uint32_t

cdef extern from "Python.h":
    ctypedef struct FILE
    ctypedef struct PyObject:
        pass
    FILE* PyFile_AsFile(object)
    PyObject* PyString_FromString(char *v)
    PyObject* PyString_FromStringAndSize(char *v, Py_ssize_t len)
    void PyEval_InitThreads()

cdef extern from "fileobject.h":
    ctypedef class __builtin__.file [object PyFileObject]:
        pass

cdef extern from "stdarg.h":
     ctypedef struct va_list:
         pass

cdef extern from "gem_tools.h" nogil:
    # general
    ctypedef int gt_status

    cdef uint64_t GT_ALL
    cdef int GT_STATUS_OK
    cdef int GT_STATUS_FAIL
    cdef int GT_STATS_INSS_FG_RANGE

    ctypedef struct gt_vector:
        pass

    #gt_string
    ctypedef struct gt_string:
        pass

    gt_string* gt_string_new(uint64_t initial_buffer_size)
    void gt_string_resize(gt_string* string,uint64_t new_buffer_size)
    void gt_string_clear(gt_string* string)
    void gt_string_delete(gt_string* string)
    char* gt_string_get_string(gt_string* string)
    uint64_t gt_string_get_length(gt_string* string)
    void gt_string_set_length(gt_string* string,uint64_t length)
    char* gt_string_char_at(gt_string* string,uint64_t pos)
    void gt_string_append_char(gt_string* string_dst,char character)
    void gt_string_append_eos(gt_string* string_dst)


    # ctypedef struct gt_file_format:
    #     pass
    # ctypedef struct gt_file_type:
    #     pass

    enum gt_file_format:
        FASTA
        MAP
        SAM
        FILE_FORMAT_UNKNOWN

    enum gt_file_type:
        STREAM
        REGULAR_FILE
        MAPPED_FILE
        GZIPPED_FILE
        BZIPPED_FILE

    # input file
    ctypedef struct gt_input_file:
        gt_file_type file_type
        gt_file_format file_format

    gt_input_file* gt_input_stream_open(FILE* stream)
    gt_input_file* gt_input_file_open(char* file_name,bool mmap_file)
    gt_status gt_input_file_close(gt_input_file* input_file)
    gt_file_format gt_input_file_detect_file_format(gt_input_file* input_file)

    enum gt_output_file_type:
        SORTED_FILE
        UNSORTED_FILE

    ctypedef struct gt_output_file:
        pass

    gt_output_file* gt_output_stream_new(FILE* file, gt_output_file_type output_file_type)
    gt_output_file* gt_output_file_new(char* file_name, gt_output_file_type output_file_type)
    gt_status gt_output_file_close(gt_output_file*  output_file)


    # buffered input
    ctypedef struct gt_buffered_input_file:
        pass
    gt_buffered_input_file* gt_buffered_input_file_new(gt_input_file* input_file)
    void gt_buffered_input_file_close(gt_buffered_input_file* input_file)
    void gt_buffered_input_file_attach_buffered_output(gt_buffered_input_file* buffered_input_file, gt_buffered_output_file* buffered_output_file)


    # buffered output
    ctypedef struct gt_buffered_output_file:
        pass
    gt_buffered_output_file* gt_buffered_output_file_new(gt_output_file* output_file)
    void gt_buffered_output_file_close(gt_buffered_output_file* buffered_output_file)

    # sam parser
    ctypedef struct gt_sam_parser_attr:
        bool sam_soap_style

    ctypedef struct gt_map_parser_attr:
        bool read_paired
        uint64_t max_parsed_maps
        gt_string* src_text

    # generic parser
    ctypedef struct gt_generic_parser_attr:
        gt_sam_parser_attr sam_parser_attr
        gt_map_parser_attr map_parser_attr


    gt_generic_parser_attr* gt_input_generic_parser_attributes_new(bool  paired_read)
    void gt_input_generic_parser_attributes_reset_defaults(gt_generic_parser_attr*  attributes)
    void gt_input_generic_parser_attributes_set_defaults(gt_generic_parser_attr*  attributes)
    bool gt_input_generic_parser_attributes_is_paired(gt_generic_parser_attr*  attributes)
    void gt_input_generic_parser_attributes_set_paired(gt_generic_parser_attr*  attributes, bool is_paired)


    gt_status gt_input_generic_parser_get_alignment(gt_buffered_input_file* buffered_input,gt_alignment* alignment, gt_generic_parser_attr* attributes)
    gt_status gt_input_generic_parser_get_template(gt_buffered_input_file* buffered_input,gt_template* template,gt_generic_parser_attr* attributes)

    ## map template parser
    gt_status gt_input_map_parse_template(char* string, gt_template* template)

    # alignment
    ctypedef struct gt_alignment:
        pass
    gt_alignment* gt_alignment_new()
    void gt_alignment_clear(gt_alignment* alignment)
    void gt_alignment_delete(gt_alignment* alignment)

    char* gt_alignment_get_tag(gt_alignment* alignment)
    void gt_alignment_set_tag(gt_alignment* alignment,char* tag,uint64_t length)
    uint64_t gt_alignment_get_tag_length(gt_alignment* alignment)

    char* gt_alignment_get_read(gt_alignment* alignment)
    void gt_alignment_set_read(gt_alignment* alignment,char* read,uint64_t length)
    uint64_t gt_alignment_get_read_length(gt_alignment* alignment)

    char* gt_alignment_get_qualities(gt_alignment* alignment)
    void gt_alignment_set_qualities(gt_alignment* alignment,char* qualities,uint64_t length)
    bool gt_alignment_has_qualities(gt_alignment* alignment)

    gt_vector* gt_alignment_get_counters_vector(gt_alignment* alignment)
    void gt_alignment_set_counters_vector(gt_alignment* alignment,gt_vector* counters)
    uint64_t gt_alignment_get_num_counters(gt_alignment* alignment)
    uint64_t gt_alignment_get_counter(gt_alignment* alignment,uint64_t stratum)
    void gt_alignment_set_counter(gt_alignment* alignment,uint64_t stratum,uint64_t value)
    void gt_alignment_dec_counter(gt_alignment* alignment,uint64_t stratum)
    void gt_alignment_inc_counter(gt_alignment* alignment,uint64_t stratum)
    int64_t gt_alignment_get_pair(gt_alignment* alignment)

    cdef char* GT_ATTR_MAX_COMPLETE_STRATA "MCS"
    cdef char* GT_ATTR_NOT_UNIQUE "NOT-UNIQUE"

    uint64_t gt_alignment_get_mcs(gt_alignment* alignment)
    void gt_alignment_set_mcs(gt_alignment* alignment,uint64_t max_complete_strata)
    void gt_alignment_set_not_unique_flag(gt_alignment* alignment,bool is_not_unique)
    bool gt_alignment_get_not_unique_flag(gt_alignment* alignment)

    uint64_t gt_alignment_get_num_maps(gt_alignment* alignment)
    void gt_alignment_add_map(gt_alignment* alignment,gt_map* map)
    void gt_alignment_add_map_gt_vector(gt_alignment* alignment,gt_vector* map_vector)
    gt_map* gt_alignment_get_map(gt_alignment* alignment, uint64_t position)
    void gt_alignment_set_map(gt_alignment* alignment,gt_map* map,uint64_t position)
    void gt_alignment_clear_maps(gt_alignment* alignment)

    # map
    #
    ctypedef struct gt_map:
        pass
    enum gt_strand:
        FORWARD
        REVERSE

    enum gt_junction_t:
        NO_JUNCTION
        SPLICE
        POSITIVE_SKIP
        NEGATIVE_SKIP
        INSERT
        JUNCTION_UNKNOWN

    gt_map* gt_map_new()
    void gt_map_delete(gt_map* map)
    char* gt_map_get_seq_name(gt_map* map)
    gt_strand gt_map_get_strand(gt_map* map)

    # template
    ctypedef struct gt_template:
        pass
    gt_template* gt_template_new()
    void gt_template_delete(gt_template* template)
    char* gt_template_get_tag(gt_template* template)
    void gt_template_set_tag(gt_template* template, char* tag, uint64_t length)
    uint64_t gt_template_get_num_blocks(gt_template* template)
    uint64_t gt_template_get_num_mmaps(gt_template* template)
    uint64_t gt_template_get_num_counters(gt_template*  template)
    uint64_t gt_template_get_counter(gt_template*  template, uint64_t stratum)
    void gt_template_set_counter(gt_template*  template, uint64_t stratum, uint64_t value)
    void gt_template_dec_counter(gt_template*  template, uint64_t stratum)
    void gt_template_inc_counter(gt_template*  template, uint64_t stratum)

    uint64_t gt_template_get_mcs(gt_template*  template)
    void gt_template_set_mcs(gt_template*  template, uint64_t max_complete_strata)
    bool gt_template_has_qualities(gt_template*  template)
    bool gt_template_get_not_unique_flag(gt_template*  template)
    void gt_template_set_not_unique_flag(gt_template* template,bool is_not_unique)
    gt_alignment* gt_template_get_block(gt_template* template, uint64_t position)
    int64_t gt_template_get_pair(gt_template* template)
    # template utils
    bool gt_template_is_mapped(gt_template* template)
    bool gt_template_is_thresholded_mapped(gt_template* template, uint64_t max_allowed_strata)
    void gt_template_trim(gt_template* template, uint64_t left, uint64_t right, uint64_t min_length, bool set_extra)

    # template merge
    cdef gt_template* gt_template_union_template_mmaps(gt_template* src_A, gt_template* src_B)


    # output printer support
    enum gt_file_fasta_format:
        F_FASTA
        F_FASTQ
        F_MULTI_FASTA



    # map output attributes
    ctypedef struct gt_output_map_attributes:
        bool print_scores
        uint64_t max_printable_maps
        bool print_extra
        bool print_casava

    gt_output_map_attributes* gt_output_map_attributes_new()
    void gt_output_map_attributes_delete(gt_output_map_attributes* attributes)
    void gt_output_map_attributes_reset_defaults(gt_output_map_attributes* attributes)
    bool gt_output_map_attributes_is_print_scores(gt_output_map_attributes* attributes)
    void gt_output_map_attributes_set_print_scores(gt_output_map_attributes* attributes, bool print_scores)
    bool gt_output_map_attributes_is_print_extra(gt_output_map_attributes* attributes)
    void gt_output_map_attributes_set_print_extra(gt_output_map_attributes* attributes, bool print_extra)
    bool gt_output_map_attributes_is_print_casava(gt_output_map_attributes* attributes)
    void gt_output_map_attributes_set_print_casava(gt_output_map_attributes* attributes, bool print_casava)
    uint64_t gt_output_map_attributes_get_max_printable_maps(gt_output_map_attributes* attributes)
    void gt_output_map_attributes_set_max_printable_maps(gt_output_map_attributes* attributes, uint64_t max_printable_maps)

    ctypedef struct gt_fasta_file_format:
        pass

    ctypedef struct gt_output_fasta_attributes:
        bool print_extra
        bool print_casava
        gt_fasta_file_format* format

    gt_output_fasta_attributes* gt_output_fasta_attributes_new()
    void gt_output_fasta_attributes_delete(gt_output_fasta_attributes* attributes)
    void gt_output_fasta_attributes_reset_defaults(gt_output_fasta_attributes* attributes)
    bool gt_output_fasta_attributes_is_print_extra(gt_output_fasta_attributes* attributes)
    void gt_output_fasta_attributes_set_print_extra(gt_output_fasta_attributes* attributes, bool print_extra)
    bool gt_output_fasta_attributes_is_print_casava(gt_output_fasta_attributes* attributes)
    void gt_output_fasta_attributes_set_print_casava(gt_output_fasta_attributes* attributes, bool print_casava)
    gt_file_fasta_format gt_output_fasta_attributes_get_format(gt_output_fasta_attributes* attributes)
    void gt_output_fasta_attributes_set_format(gt_output_fasta_attributes* attributes, gt_file_fasta_format format)

    ## print fastq/fasta
    gt_status gt_output_fasta_bofprint_alignment(gt_buffered_output_file* buffered_output_file,gt_alignment* alignment, gt_output_fasta_attributes* output_attributes)
    gt_status gt_output_fasta_bofprint_template(gt_buffered_output_file* buffered_output_file,gt_template* template, gt_output_fasta_attributes* output_attributes)
    gt_status gt_output_fasta_sprint_template(gt_string* string,gt_template* template, gt_output_fasta_attributes* output_attributes)
    gt_status gt_output_fasta_ofprint_template(gt_output_file* output_file,gt_template* template, gt_output_fasta_attributes* attributes)
    gt_status gt_output_fasta_sprint_alignment(gt_string* string,gt_alignment* alignment, gt_output_fasta_attributes* output_attributes)

    ## print map
    gt_status gt_output_map_bofprint_template(gt_buffered_output_file* buffered_output_file,gt_template* template,gt_output_map_attributes* attributes)
    gt_status gt_output_map_bofprint_alignment(gt_buffered_output_file* buffered_output_file,gt_alignment* alignment,gt_output_map_attributes* attributes)
    gt_status gt_output_map_sprint_template(gt_string* string,gt_template* template,gt_output_map_attributes* attributes)
    gt_status gt_output_map_sprint_alignment(gt_string* string,gt_alignment* alignment,gt_output_map_attributes* attributes)
    gt_status gt_output_map_ofprint_template(gt_output_file* output_file,gt_template* template, gt_output_map_attributes* attributes)

cdef extern from "gt_stats.h":
    ctypedef struct gt_maps_profile:
        uint64_t *mismatches
        uint64_t *levenshtein
        uint64_t *insertion_length
        uint64_t *deletion_length
        uint64_t *errors_events
        uint64_t total_mismatches
        uint64_t total_levenshtein
        uint64_t total_indel_length
        uint64_t total_errors_events
        uint64_t *error_position

        uint64_t total_bases
        uint64_t total_bases_matching
        uint64_t total_bases_trimmed

        uint64_t *inss
        uint64_t *inss_fine_grain

        uint64_t *misms_transition
        uint64_t *qual_score_misms
        uint64_t *misms_1context
        uint64_t *misms_2context
        uint64_t *indel_transition_1
        uint64_t *indel_transition_2
        uint64_t *indel_transition_3
        uint64_t *indel_transition_4
        uint64_t *indel_1context
        uint64_t *indel_2context
        uint64_t *qual_score_errors


    ctypedef struct gt_splitmaps_profile:
        uint64_t num_mapped_with_splitmaps
        uint64_t num_mapped_only_splitmaps
        uint64_t total_splitmaps
        uint64_t total_junctions
        uint64_t *num_junctions # NUM_JUNCTION_RANGE */
        uint64_t *length_junctions # LEN_JUNCTION_RANGE */
        uint64_t *junction_position # READ_POS_RANGE */
        # Paired SM combinations */
        uint64_t pe_sm_sm
        uint64_t pe_sm_rm
        uint64_t pe_rm_rm

    ctypedef struct gt_stats:
        uint64_t min_length
        uint64_t max_length
        uint64_t total_bases
        uint64_t total_bases_aligned
        uint64_t mapped_min_length
        uint64_t mapped_max_length
        uint64_t *nt_counting
        uint64_t num_blocks
        uint64_t num_alignments
        uint64_t num_maps
        uint64_t num_mapped
        uint64_t *mmap
        uint64_t *uniq
        gt_maps_profile *maps_profile
        gt_splitmaps_profile* splitmaps_profile

    cdef gt_stats* gt_stats_new()
    cdef void gt_stats_clear(gt_stats *stats)
    cdef void gt_stats_delete(gt_stats *stats)

    void gt_stats_merge(gt_stats** stats, uint64_t stats_array_size)
    void gt_stats_calculate_template_stats(gt_stats*  stats, gt_template* template, bool best_map)
    # stats ranges
    cdef int GT_STATS_MMAP_RANGE
    cdef int GT_STATS_INSS_RANGE
    cdef int GT_STATS_MISMS_RANGE
    cdef int GT_STATS_UNIQ_RANGE
    cdef int GT_STATS_LARGE_READ_POS_RANGE
    cdef int GT_STATS_SHORT_READ_POS_RANGE
    cdef int GT_STATS_MISMS_BASE_RANGE
    cdef int GT_STATS_NT_BASE_RANGE
    cdef int GT_STATS_QUAL_SCORE_RANGE
    cdef int GT_STATS_NUM_JUNCTION_RANGE
    cdef int GT_STATS_LEN_JUNCTION_RANGE
    cdef int GT_STATS_MISMS_1_CONTEXT_RANGE
    cdef int GT_STATS_MISMS_2_CONTEXT_RANGE
    cdef int GT_STATS_INDEL_TRANSITION_1_RANGE
    cdef int GT_STATS_INDEL_TRANSITION_2_RANGE
    cdef int GT_STATS_INDEL_TRANSITION_3_RANGE
    cdef int GT_STATS_INDEL_TRANSITION_4_RANGE
    cdef int GT_STATS_INDEL_1_CONTEXT
    cdef int GT_STATS_INDEL_2_CONTEXT


cdef extern from "gemtools_binding.h" nogil:
    void gt_merge_files_synch(gt_output_file* output_file, uint64_t threads, uint64_t num_files, gt_input_file** files)
    void gt_write_stream(gt_output_file* output, gt_input_file** inputs, uint64_t num_inputs, bool append_extra, bool clean_id, bool interleave, uint64_t threads, bool write_map)
    void gt_stats_fill(gt_input_file* input_file, gt_stats* target_all_stats, gt_stats* target_best_stats, uint64_t num_threads, bool paired_end)
    void gt_stats_print_stats(FILE* output, gt_stats* stats, bool paired_end)