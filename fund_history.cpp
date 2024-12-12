//*********************************************************************************
//  fund_history - do some computations on Fidelity Purchase History page
//  build: g++ -Wall -O2 fund_history.cpp -o fund_history.exe
//  test run: fund_history -fFFRHX "Portfolio Positions.htm"
//*********************************************************************************
#include <windows.h>
#include <stdio.h>
#ifdef _lint
#include <stdlib.h>
#endif
#include <io.h>
// #include <fcntl.h>
// #include <sys/types.h>

typedef  unsigned int  uint ;

//lint -e10   Expecting '}'

//lint -e641  PSF_SEEK_FUND_CODE, PSF_SEEK_ACQUIRED, PSF_FOUND_ACQUIRED, PSF_SCAN_DATA, PSF_END)
//lint -e818  Pointer parameter could be declared as pointing to const
//lint -esym(40, errno)
//lint -esym(749, PSF_END)  //local enumeration constant not referenced

static char fund_data[MAX_PATH+1] = "" ;
#define  FUND_NAME_LEN  5
static char fund_code[FUND_NAME_LEN+1] = "";

//****************************************************************************
void usage(void)
{
   puts("Usage: fund_history fund_data_filename -f<five_char_fund>");
}

//****************************************************************************
//  state 0: search for <scan>FCODE</scan>
//  state 1: search for 'Acquired' in data table header
//  state 2: search for first data row [pvd-table__row], iterate over these rows
//  state 3: iterate over data elements in this row
static char fund_str[80] ;
static uint parse_state = 0 ;

enum {
   PSF_SEEK_FUND_CODE=0,
   PSF_SEEK_ACQUIRED,
   PSF_FOUND_ACQUIRED,
   PSF_SCAN_DATA,
   PSF_END   
} ;

//****************************************************************************
//  data table header
// <table class="pvd-table__table posweb-purchase-history" tabindex="0">
// <thead class="pvd-table__head">
// <tr class="pvd-table__row posweb-lots-table-row-header">
// <th scope="col" class="pvd-table__column-header-cell posweb-lots-header">
// <span class="pvd-table__column-header-cell-content">Acquired</span></th>
// <th scope="col" class="pvd-table__column-header-cell posweb-lots-header">
// <span class="pvd-table__column-header-cell-content">Term</span></th>
// <th scope="col" class="pvd-table__column-header-cell posweb-lots-header">
// <span class="pvd-table__column-header-cell-content">$ Total Gain/Loss</span></th><th scope="col" class="pvd-table__column-header-cell posweb-lots-header"><span class="pvd-table__column-header-cell-content">% Total Gain/Loss</span></th><th scope="col" class="pvd-table__column-header-cell posweb-lots-header"><span class="pvd-table__column-header-cell-content">Current Value</span></th><th scope="col" class="pvd-table__column-header-cell posweb-lots-header">
// <span class="pvd-table__column-header-cell-content">Quantity</span></th><th scope="col" class="pvd-table__column-header-cell posweb-lots-header">
// <span class="pvd-table__column-header-cell-content">Average Cost Basis</span></th><th scope="col" class="pvd-table__column-header-cell posweb-lots-header"><span class="pvd-table__column-header-cell-content">Cost Basis Total</span></th>
// </tr></thead>
// <tbody class="pvd-table__body">
//  
//  Data rows
// <tr class="pvd-table__row posweb-lots-table-row">
// <td class="pvd-table__data-cell pvd-table__data-cell--right">Nov-29-2024</td>
// <td class="pvd-table__data-cell">Short</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right posweb-cell-loss">-$1.71</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right posweb-cell-loss">-0.41%</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right">$417.15</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right">44.663</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right">$9.38</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right">$418.86</td>
// </tr>
// 
// <tr class="pvd-table__row posweb-lots-table-row">
// <td class="pvd-table__data-cell pvd-table__data-cell--right">Oct-31-2024</td>
// <td class="pvd-table__data-cell">Short</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right posweb-cell-loss">-$1.81</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right posweb-cell-loss">-0.41%</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right">$444.43</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right">47.583</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right">$9.38</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right">$446.24</td>
// </tr>
//  last table row
//  $55,027.22</td></tr></tbody></table>

//****************************************************************************
static char *seek_next_tr_tag(char *hd)
{
   char *tl = strstr(hd, "posweb-lots-table-row") ;
   if (tl != NULL) {
      tl = strchr(tl, '>');
   }
   return tl ;
}

//****************************************************************************
//  $55,027.22</td
static double convert_to_double(char *td)
{
   char tbuf[30] = "" ;
   uint outidx = 0 ;
   while (*td != 0) {
      switch (*td) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case '.':
         tbuf[outidx++] = *td ;
         tbuf[outidx] = 0 ;
         break ;
         
      default:
         break ;
      }
      
      td++ ;
   }
   double tdbl = strtod(tbuf, NULL);   //lint !e119
   // printf("      [%.2f]\n", tdbl);
   return tdbl ;
}

//****************************************************************************
//  Acquired has been Acquired...
//  All required data is in the string that is passed to this function.
//  Now, find the required elements and process them.
//****************************************************************************
static double row4sum = 0.0 ;
static double row7sum = 0.0 ;

static int parse_fund_data(char *inpstr)
{
   char *srch = NULL;
   
   row4sum = 0.0 ;
   row7sum = 0.0 ;
   uint data_column = 0 ;
   
   bool outer_done = false ;  //  parse entire table
   while (!outer_done) {
      switch (parse_state) {
      case PSF_FOUND_ACQUIRED:
         srch = strstr(inpstr, "pvd-table__body");
         //  seek end of data header row
         if (srch == NULL) {
            printf("end of table header line not found, aborting!!\n");
            return 1 ;
         }
         printf("found start of data table\n");
         parse_state = PSF_SCAN_DATA ; //  go to iterate-over-table-row state
         break ;
         
      //  iterate over table-row entries
// <tr class="pvd-table__row posweb-lots-table-row">
// <td class="pvd-table__data-cell pvd-table__data-cell--right">Oct-31-2024</td>
// <td class="pvd-table__data-cell">Short</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right posweb-cell-loss">-$1.81</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right posweb-cell-loss">-0.41%</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right">$444.43</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right">47.583</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right">$9.38</td>
// <td class="pvd-table__data-cell pvd-table__data-cell--right">$446.24</td>
// </tr>

#define  DEBUG_OUTPUT
      case PSF_SCAN_DATA:
         {  //  begin local context
         printf("ready to start scanning data elements\n");
         uint data_rows = 0 ;
         char *tr = srch ;
         //  iterate over line with all table data in it
         bool done = false ;
         while (!done) {   //  parse table data
            data_rows++ ;
#ifdef  DEBUG_OUTPUT
            printf("data row: %u\n", data_rows);
#endif
            data_column = 0 ;
            //  seek to <tr> tag for next row
            tr = seek_next_tr_tag(tr);
            if (tr == NULL) {
               printf("data rows: %u, end of table found\n", data_rows);
               parse_state = PSF_SEEK_FUND_CODE ;
               done = true ;
               outer_done = true ;
               continue ;
            }
            //  temp null-term row tag so TD string searches will be bounded
            char *trcl = strstr(tr, "</tr>");
            if (trcl == NULL) {
               printf("DR: %u, row term not found\n", data_rows);
               parse_state = PSF_SEEK_FUND_CODE ;
               done = true ;
               outer_done = true ;
               continue ;
            }
            *trcl = 0 ; //  NULL-term the row close tag
            
            
            //*****************************************************
            //  iterate over TD elements in row
            //*****************************************************
            char *td = tr ;
            bool data_srch_done = false ;
            while (!data_srch_done) {
               // printf("### TR len left: %u\n", strlen(td));
               td = strstr(td, "<td") ;
               //  if this fails, we likely are at end of row
               if (td == NULL) {
                  // printf("DR: %u, TD open not found\n", data_rows);
                  // parse_state = PSF_SEEK_FUND_CODE ;
                  data_srch_done = true ;
                  // done = true ;
                  // outer_done = true ;
                  continue ;
               }
               td = strchr(td, '>');
               if (td == NULL) {
                  printf("DR: %u, TD open tail not found\n", data_rows);
                  parse_state = PSF_SEEK_FUND_CODE ;
                  data_srch_done = true ;
                  done = true ;
                  outer_done = true ;
                  continue ;
               }
               td++ ;   //  point to data element
               //  debug: temp null-term TD close tag for debug output
               char *tdcl = strstr(td, "</td>");
               if (tdcl == NULL) {
                  printf("DR: %u, TD close not found\n", data_rows);
                  parse_state = PSF_SEEK_FUND_CODE ;
                  data_srch_done = true ;
                  done = true ;
                  outer_done = true ;
                  continue ;
               }
               tdcl += 4 ; //  skip past TD close tag
               *tdcl = 0 ;
               printf("   %u: %s\n", data_column, td);
               switch (data_column) {
               case 4:
                  row4sum += convert_to_double(td) ;
                  break ;
               case 7:
                  row7sum += convert_to_double(td) ;
                  break ;
                  
               default:
                  break ;
               }
               *tdcl = '>' ;  //  restore TD close tag
               data_column++ ;
            }  //  end !data_srch_done
            
            
            *trcl = '<' ;  //  restore row term after parsing data elements
            tr = trcl ;   //  slide tr ptr to next element
            //  now, iterate over <td> tags until </tr> is found
         }
         }  //  end local context
         break ;
         
      default: 
         printf("unused deep state, resetting...\n");
         parse_state = PSF_SEEK_FUND_CODE ;
         outer_done = true ;
         break ;
      }
   }
   return 0 ;
}
         
//****************************************************************************
static int parse_input_line(char *inpstr, uint lcount, uint slen)
{
   //  states 1, 2, 3 are all in one common line of text
// L2135  slen: 251
// L2136  slen: 28254
// L2137  slen: 95
//    if (lcount > 2130  &&  lcount < 2150) {
//       printf("L%u  slen: %u\n", lcount, slen);
//    }
   int result = 0 ;
   switch (parse_state) {
   case PSF_SEEK_FUND_CODE:  //  search for <scan>FCODE</scan>
      if (strstr(inpstr, fund_str) != NULL) {
         printf("Found %s\n", fund_code);
         parse_state = PSF_SEEK_ACQUIRED ;
      }
      break ;
      
   case PSF_SEEK_ACQUIRED:
      if (strstr(inpstr, "Acquired") != NULL) {
         //  all table data will be in this one line
         printf("Line %u, length: %u: Found [Acquired]\n", lcount, slen);
         parse_state = PSF_FOUND_ACQUIRED ;
         result = parse_fund_data(inpstr);
      }
      break ;
      
   default:
      printf("unused state, resetting...\n");
      parse_state = PSF_SEEK_FUND_CODE ;
      break ;
   }
   return result;
}

//****************************************************************************
int main(int argc, char** argv)
{
   int idx ;
   for (idx=1; idx<argc; idx++) {
      char *p = argv[idx];
      if (*p == '-') {
         p++ ;
         switch (*p) {
         case 'f':
            p++ ;
            strncpy(fund_code, p, FUND_NAME_LEN);
            fund_code[FUND_NAME_LEN] = 0 ;
            break ;
            
         default:
            usage() ;
            return 1 ;
         }
      }
      else {
         strncpy(fund_data, p, MAX_PATH);
         fund_data[MAX_PATH] = 0 ;
      }
   }
   
   if (fund_data[0] == 0  ||  fund_code[0] == 0) {
      usage() ;
      return 1 ;
   }
   
   printf("seek %s in %s\n", fund_code, fund_data);
   sprintf(fund_str, "<span>%s</span>", fund_code);

   FILE* infd = fopen(fund_data, "rt");
   if (infd == NULL) {
      printf("%s: %s\n", fund_data, strerror(errno));
      return errno;
   }
   
//  the longest line that I saw in my first reference html file, was 15,999 bytes   
#define  MAX_LINE_LEN   (1024*1024)
#define  MAX_SAFE_LEN   (MAX_LINE_LEN - 2000)
   char inpstr[MAX_LINE_LEN+1];
   int result = 0 ;
   uint lcount = 0 ;
   uint max_line_len = 0 ;
   while (fgets(inpstr, MAX_LINE_LEN, infd) != NULL) {
      lcount++ ;
      uint slen = strlen(inpstr);
      if (slen > MAX_SAFE_LEN) {
         printf("line %u: excessive line length [%u]\n", lcount, slen);
         printf("test file with maxlen, then update this program");
         result = 1 ;
         break ;
      }
      if (max_line_len < slen) {
         max_line_len = slen ;
      }
      
      result = parse_input_line(inpstr, lcount, slen);
      if (result != 0) {
         printf("line %u: error %d\n", lcount, result);
         break ;
      }
   }
   fclose(infd);
   printf("max line length: %u chars\n", max_line_len);
   printf("Current value:    %.2f\n", row4sum);
   printf("Cost basis total: %.2f\n", row7sum);
   
   return result ;
}
