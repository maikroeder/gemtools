/*
 * PROJECT: GEM-Tools library
 * FILE: gt.filter.c
 * DATE: 02/08/2012
 * AUTHOR(S): Santiago Marco-Sola <santiagomsola@gmail.com>
 * DESCRIPTION: Application to filter {MAP,SAM,FASTQ} files and output the filtered result
 */

#include <getopt.h>
#include <omp.h>

#include "gem_tools.h"

typedef struct {
  /* I/O */
  char* name_input_file;
  char* name_output_file;
  char* name_reference_file;
  bool mmap_input;
  bool paired_end;
  /* Filter */
  bool mapped;
  bool unmapped;
  bool no_split_maps;
  bool only_split_maps;
  bool best_map;
  uint64_t max_matches;
  bool make_counters;
  bool only_unmapped;
  bool only_mapped;
  /* Filter-Realign */
  bool realign_hamming;
  bool realign_levenshtein;
  /* Hidden */
  bool error_plot;
  bool insert_size_plot;
  /* Misc */
  uint64_t num_threads;
  bool verbose;
} gt_stats_args;

gt_stats_args parameters = {
    /* I/O */
    .name_input_file=NULL,
    .name_output_file=NULL,
    .name_reference_file=NULL,
    .mmap_input=false,
    .paired_end=false,
    /* Filter */
    .mapped=false,
    .unmapped=false,
    .no_split_maps=false,
    .only_split_maps=false,
    .best_map=false,
    .max_matches=GT_ALL,
    .make_counters=false,
    .only_unmapped=false,
    .only_mapped=false,
    /* Filter-Realign */
    .realign_hamming=false,
    .realign_levenshtein=false,
    /* Hidden */
    .error_plot = false,
    .insert_size_plot = false,
    /* Misc */
    .num_threads=1,
    .verbose=false,
};

void gt_template_filter(gt_template *template_dst,gt_template *template_src) {
  GT_TEMPLATE_IF_REDUCES_TO_ALINGMENT(template_src,alignment_src) {
    GT_TEMPLATE_REDUCTION(template_dst,alignment_dst);
    GT_ALIGNMENT_ITERATE(alignment_src,map) {
      register const uint64_t num_blocks = gt_map_get_num_blocks(map);
      if (parameters.no_split_maps && num_blocks>1) continue;
      if (parameters.only_split_maps && num_blocks==1) continue;
      gt_alignment_insert_map(alignment_dst,gt_map_copy(map));
      if (parameters.best_map) return;
    }
  } GT_TEMPLATE_END_REDUCTION__RETURN;
  register const uint64_t num_blocks = gt_template_get_num_blocks(template_src);
  GT_TEMPLATE__ATTR_ITERATE(template_src,mmap,mmap_attr) {
    // Check SM contained
    register uint64_t has_sm = false;
    if (parameters.no_split_maps || parameters.only_split_maps) {
      GT_MULTIMAP_ITERATE(mmap,map,end_p) {
        if (gt_map_get_num_blocks(map)>1) {
          has_sm = true; break;
        }
      }
    }
    if (parameters.no_split_maps && has_sm) continue;
    if (parameters.only_split_maps && !has_sm) continue;
    // Add the mmap
    register gt_map** mmap_copy = gt_mmap_array_copy(mmap,num_blocks);
    gt_template_add_mmap(template_dst,mmap_copy,mmap_attr);
    free(mmap_copy);
    // Skip the rest if best
    if (parameters.best_map) return;
  }
}

void gt_filter_read__write() {
  // Open file IN/OUT
  gt_input_file* input_file = (parameters.name_input_file==NULL) ?
      gt_input_stream_open(stdin) : gt_input_file_open(parameters.name_input_file,parameters.mmap_input);
  gt_output_file* output_file = (parameters.name_output_file==NULL) ?
      gt_output_stream_new(stdout,SORTED_FILE) : gt_output_file_new(parameters.name_output_file,SORTED_FILE);

  // Open reference file
  register gt_sequence_archive* sequence_archive = NULL;
  if (parameters.name_reference_file!=NULL && (parameters.realign_hamming || parameters.realign_levenshtein)) {
    sequence_archive = gt_sequence_archive_new();
    register gt_input_file* const reference_file = gt_input_file_open(parameters.name_reference_file,false);
    fprintf(stderr,"Loading reference file ...");
    if (gt_input_multifasta_parser_get_archive(reference_file,sequence_archive)!=GT_IFP_OK) {
      fprintf(stderr,"\n");
      gt_fatal_error_msg("Error parsing reference file '%s'\n",parameters.name_reference_file);
    }
    gt_input_file_close(reference_file);
    fprintf(stderr," done! \n");
  }

  // Parallel reading+process
  #pragma omp parallel num_threads(parameters.num_threads)
  {
    gt_status error_code;
    gt_buffered_input_file* buffered_input = gt_buffered_input_file_new(input_file);
    gt_buffered_output_file* buffered_output = gt_buffered_output_file_new(output_file);
    gt_buffered_input_file_attach_buffered_output(buffered_input,buffered_output);
    gt_generic_parser_attr generic_parser_attr = GENERIC_PARSER_ATTR_DEFAULT(parameters.paired_end);
    gt_output_map_attributes output_attributes = GT_OUTPUT_MAP_ATTR_DEFAULT();

    // Limit max-matches
    generic_parser_attr.map_parser_attr.max_parsed_maps = parameters.max_matches;

    gt_template* template = gt_template_new();
    while ((error_code=gt_input_generic_parser_get_template(buffered_input,template,&generic_parser_attr))) {
      if (error_code!=GT_IMP_OK) {
        gt_error_msg("Fatal error parsing file '%s':%"PRIu64"\n",parameters.name_input_file,buffered_input->current_line_num-1);
      }

      // Consider mapped/unmapped
      register const bool is_mapped = gt_template_is_mapped(template);
      if (parameters.mapped && !is_mapped) continue;
      if (parameters.unmapped && is_mapped) continue;

      // Hidden options (aborts the rest)
      if (parameters.error_plot || parameters.insert_size_plot) {
        if (parameters.error_plot) {
          if (parameters.best_map)  {
            uint64_t best_distance = UINT64_MAX;
            GT_TEMPLATE_ITERATE_(template,mmap) {
              register const uint64_t dist = gt_map_get_global_levenshtein_distance(*mmap);
              if (dist < best_distance) best_distance = dist;
            }
            if (best_distance < UINT64_MAX) fprintf(stdout,"%lu\n",best_distance);
          } else {
            GT_TEMPLATE_ITERATE_(template,mmap) {
              fprintf(stdout,"%lu\n",gt_map_get_global_levenshtein_distance(*mmap));
            }
          }
        } else if (parameters.insert_size_plot && gt_template_get_num_blocks(template)>1) {
          uint64_t error_code;
          GT_TEMPLATE_ITERATE_(template,mmap) {
            fprintf(stdout,"%lu\n",gt_template_get_insert_size(mmap,&error_code));
            if (parameters.best_map) break;
          }
        }
      } else {
        // First realign
        if (parameters.realign_levenshtein) {
          gt_template_realign_levenshtein(template,sequence_archive);
        } else if (parameters.realign_hamming) {
          gt_template_realign_hamming(template,sequence_archive);
        }

        // Pick up best-map || erase splitmaps
        if (parameters.best_map || parameters.no_split_maps || parameters.only_split_maps) {
          gt_template *template_best = gt_template_copy(template,false,false);
          gt_template_filter(template_best,template);
          gt_template_delete(template);
          template = template_best;
        }

        // Make counters
        if (parameters.make_counters) {
          gt_template_recalculate_counters(template);
        }

        // Print template
        gt_output_map_bofprint_template(buffered_output,template,&output_attributes);
      }
    }

    // Clean
    gt_template_delete(template);
    gt_buffered_input_file_close(buffered_input);
    gt_buffered_output_file_close(buffered_output);
    if (sequence_archive != NULL) gt_sequence_archive_delete(sequence_archive);
  }

  // Clean
  gt_input_file_close(input_file);
  gt_output_file_close(output_file);
}

void usage() {
  fprintf(stderr, "USE: ./gt.filter [ARGS]...\n"
                  "         [I/O]\n"
                  "           --input|-i [FILE]\n"
                  "           --output|-o [FILE]\n"
                  "           --reference|-r [FILE]\n"
                  "           --mmap-input\n"
                  "           --paired-end|p\n"
                  "         [Filter]\n"
                  "           --unmapped|--mapped\n"
                  "           --no-split-maps|--only-split-maps\n"
                  "           --best-map\n"
                  "           --max-matches <number>\n"
                  "           --make-counters <number>\n"
                  "         [Filter-Realign]\n"
                  "           --hamming-realign\n"
                  "           --levenshtein-realign\n"
                  "         [Output]\n"
                  "           --display-pretty\n"
                  "         [Misc]\n"
                  "           --threads|t\n"
                  "           --verbose|v\n"
                  "           --help|h\n");
}

void parse_arguments(int argc,char** argv) {
  struct option long_options[] = {
    /* I/O */
    { "input", required_argument, 0, 'i' },
    { "output", required_argument, 0, 'o' },
    { "reference", required_argument, 0, 'r' },
    { "mmap-input", no_argument, 0, 1 },
    { "paired-end", no_argument, 0, 'p' },
    /* Filter */
    { "mapped", no_argument, 0, 2 },
    { "unmapped", no_argument, 0, 3 },
    { "no-split-maps", no_argument, 0, 4 },
    { "only-split-maps", no_argument, 0, 5 },
    { "best-map", no_argument, 0, 6 },
    { "max-matches", required_argument, 0, 7 },
    { "make-counters", no_argument, 0, 8 },
    { "hamming-realign", no_argument, 0, 9 },
    { "levenshtein-realign", no_argument, 0, 10 },
    /* Hidden */
    { "error-plot", no_argument, 0, 20 },
    { "insert-size-plot", no_argument, 0, 21 },
    /* Misc */
    { "threads", required_argument, 0, 't' },
    { "verbose", no_argument, 0, 'v' },
    { "help", no_argument, 0, 'h' },
    { 0, 0, 0, 0 } };
  int c,option_index;
  while (1) {
    c=getopt_long(argc,argv,"i:o:r:pt:hv",long_options,&option_index);
    if (c==-1) break;
    switch (c) {
    /* I/O */
    case 'i':
      parameters.name_input_file = optarg;
      break;
    case 'o':
      parameters.name_output_file = optarg;
      break;
    case 'r':
      parameters.name_reference_file = optarg;
      break;
    case 1:
      parameters.mmap_input = true;
      break;
    case 'p':
      parameters.paired_end = true;
      break;
    /* Filter */
    case 2: // mapped
      parameters.mapped = true;
      break;
    case 3: // unmapped
      parameters.unmapped = true;
      break;
    case 4: // no-split-maps
      parameters.no_split_maps = true;
      break;
    case 5: // only-split-maps
      parameters.only_split_maps = true;
      break;
    case 6:
      parameters.best_map = true;
      break;
    case 7:
      parameters.max_matches = atoll(optarg);
      break;
    case 8:
      parameters.make_counters = true;
      break;
    case 9:
      parameters.realign_hamming = true;
      break;
    case 10:
      parameters.realign_levenshtein = true;
      break;
    /* Hidden */
    case 20:
      parameters.error_plot = true;
      break;
    case 21:
      parameters.insert_size_plot = true;
      break;
    /* Misc */
    case 't':
      parameters.num_threads = atol(optarg);
      break;
    case 'v':
      parameters.verbose = true;
      break;
    case 'h':
      usage();
      exit(1);
    case '?':
    default:
      gt_fatal_error_msg("Option not recognized");
    }
  }
  /*
   * Parameters check
   */
  if (parameters.realign_hamming || parameters.realign_levenshtein) {
    if (parameters.name_reference_file==NULL) gt_fatal_error_msg("Reference file required to realign");
  }
}

int main(int argc,char** argv) {
  // Parsing command-line options
  parse_arguments(argc,argv);

  // Filter !
  gt_filter_read__write();

//    gt_map* map = gt_map_new();
//
//  map->base_length = 25;
//  gt_map_realign_levenshtein(
//      map,
//      "AAAAAAAAAAAAAAAAAAAAAAAAA",map->base_length,
//      "AAAA",4,false);
//
//  map->base_length = 25;
//  gt_map_realign_levenshtein(
//      map,
//      "AAAAAAAAAAAAGAAAAAAAAAAAA",map->base_length,
//      "AAAATAAAAAAAAAAAAAAAACAAA",25,false);
//
//  map->base_length = 4;
//  gt_map_realign_levenshtein(
//      map,
//      "AAAA",map->base_length,
//      "AAAAAAAAAAAAAAAAAAAAAAAAA",25,false);
//
//  map->base_length = 4;
//  gt_map_realign_levenshtein(
//      map,
//      "CCCC",map->base_length,
//      "AAAAAAAAAACCCCAAAAAAAAAAA",25,false);
//
//  map->base_length = 19;
//  gt_map_realign_levenshtein(
//      map,
//      "AAAAAAAAAAAAAAAAAAA",map->base_length,
//      "AAAAACCAAAACCCAAAAAAACAAA",25,false);
//
//  map->base_length = 25;
//  gt_map_realign_levenshtein(
//      map,
//      "AAAAAAAAAAAAAAAAAAAAAAAAA",map->base_length,
//      "AAAATAAAAAAAAAAAAAAAAAAAAC",26,false);
//
//  map->base_length = 4;
//  gt_map_realign_levenshtein(
//      map,
//      "CCCC",map->base_length,
//      "AAAAAAAAAACCCCAAAAAAAAAAA",25,true);

//  map->base_length = 363;
//  gt_map_realign_levenshtein(
//      map,
//      "TAATTGCTATATCCCTCAAACATCCTTTACCCTGAAATCCCTTCTAATCCATCCTCTGCCACTGCTTCCAGATTATTCTCTCTGAAATCAAGTCTAATCATGTCACT"
//      "TTTTAGCTTAAAATACTTCAATGGCACTCCATAGTTAACCAGACAGGAAGAAAGTAAAGCATACGGTCAAGAGTCCTGGCTCTAGAGTGAGACTGCCTGGGTTCAAA"
//      "ATCCTAGTATGACAGTTAATAAATCTTAATACCTGTGTGAACTTGGGAGGATGACTTCACTTCTCCTTTGCCTTCAGTTGCTTATCTAAATGAGTTAATGTAATGTA"
//      "AAGCACATGCCACACTGAAGTACTTTAATCAATATTAGCTGTTATTGTAAGTTCAAGTTTTGTAGTTAAATT",map->base_length,
//      "TAATTGCTATATCCCTCAAACATCCTTTACCCTGAATCCCTTCTAATCCATCCTCTGCACTGCTTCCAGATTATTCTC"
//      "TCTGAAAATCAAGTCTAATCATGTCACTTTTTAGCTTAAAATACTTCAATGGCACTCCATAGTTAACCAGACAGGAAG"
//      "AAAGTAAAGCATACGGTCAAGAGTCCTGGCTCTAGAGTGAGACTGCCTGGGTTCAAATCCTAGTATGACAGTTAATAA"
//      "ATCTTAATACCTGTGTGAACTTGGGAGGATGACTTCACTTCTCCTTTGCCTCAGTTGCTTTATCTAAATGAGTTAATG"
//      "TATGTAAAGCACATGCCACACTGAAGTACTTTAATCAATATTAGCTGTTATTGTAAGTTCAAGTTTTGTAGTTTAAAT"
//      "TCCTTAAGAAAACTCCCAAAAAACAGACGTCATATCATGATCTTGCCCCTTTCTACTACTTATGAACCTCCCCAAAGCTAT",471,true);

//  map->base_length = 363;
//  gt_map_realign_levenshtein(
//      map,
//      "TTAGATTGGGTTGGCTGGTATGCATGAAAATGACAGACCACTATAATTTTCCTTACAAAGAAAAATCTATGCAGTTGGA"
//      "TGGTTTCTTTTAAAAATGAACTAATTTTGATTATTTGCTAACTTTCCCAGCTTTATTGTCCAGAAACAATAGTCCTTGG"
//      "AATAAAGAAAATGTCAAAGAGTAAAACAAGCCAGCGCATTTAAATTGTAAGAATTATTTTTAAAAAATAAGATTGGACT"
//      "GGACTGCAATTTTATAACTGAACCACATTTAATTCTATCTTGCATGGGGTCACTGCACAACATGATTTGAGTTCTCCTT"
//      "AGAGCTTTCCCATCTCTTCCTAGGAGGCTGAAGAATTTATGGAGACA",map->base_length,
//      "CCACTTTGATTGGGTTGGCTGGTAATGCATTGAAATGACAGACACTATAATTTTCCTTACAAAGAAAAATCTATGCAGTTG"
//      "GATGGTTTCTTTAAAAATGAACTAATTTTGATTATTTGCTAACTTTCCCAGTTCTTATGTCAGAAACAATAGTCCTTGAAT"
//      "AAAGAAAATGTCAAAGAGTAAAAACAAGCCAGCGCATTTAAATTGTAGAATTATTTTTAAAAAATAAGATTGGACTGGACT"
//      "GCAATTTTATAACTGAACACATTTTAATTCTATCTTGCATGGGGTCACTGCACACATGATTTGAGTTCTCCTTAGAGCTTT"
//      "CCCATCTCTTCCTAGGAGGCTGAAGAATTTATGGAGACAAAATAGGCAAGGACATTTCTTAGAGAATATGCAATGCAGTTC"
//      "ATCCAGAATGACATCTTAGAGGTTATTTTG",435,true);

  return 0;
}

