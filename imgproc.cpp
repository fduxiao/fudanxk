#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "svm.h"
#include "jpeg/jpeglib.h"
/*for libsvm
Copyright (c) 2000-2013 Chih-Chung Chang and Chih-Jen Lin
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

3. Neither name of copyright holders nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
int print_null(const char *s,...) {return 0;}

static int (*info)(const char *fmt,...) = &printf;
const char charset[] = "2345678bcdefgmnpwxy";
struct svm_node *x;
int max_nr_attr = 64;

struct svm_model* model;
int predict_probability=0;

//static char *line = NULL;
static int max_line_len;

char sbuf[6666] = {0};
typedef struct tagRGB
{
	unsigned char B;
	unsigned char G;
	unsigned char R;
}RGB;
void predict(char *input, char* output);
int process_jpeg_file(const char *input_filename, char* str)
{
        struct jpeg_decompress_struct cinfo;
        struct jpeg_error_mgr jerr;
        FILE *input_file = stdin;
        JSAMPARRAY buffer;
        int row_width;

        unsigned char *output_buffer;
        unsigned char *tmp = NULL;
	int mn, mx, delta, has, j, i, x, y;//variants used by python
	RGB* pcl;//pointer to process pixels

        cinfo.err = jpeg_std_error(&jerr);

        jpeg_create_decompress(&cinfo);

        /* Specify data source for decompression */
        jpeg_stdio_src(&cinfo, input_file);

        /* Read file header, set default decompression parameters */
        (void) jpeg_read_header(&cinfo, TRUE);

        /* Start decompressor */
        (void) jpeg_start_decompress(&cinfo);

        row_width = cinfo.output_width * cinfo.output_components;

        buffer = (*cinfo.mem->alloc_sarray)
                ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_width, 1);

	output_buffer = (unsigned char *)malloc(row_width * cinfo.output_height);
        memset(output_buffer, 0, row_width * cinfo.output_height);
        tmp = output_buffer;
	//printf("width:%d height:%d\n", cinfo.output_width, cinfo.output_height);
        /* Process data */
        while (cinfo.output_scanline < cinfo.output_height) 
	{
                jpeg_read_scanlines(&cinfo, buffer, 1);
                memcpy(tmp, *buffer, row_width);
                tmp += row_width;
        }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/****************************************此处处理像素点，从cyalice.py里翻译的*************************************************************/
	for(i = 0; i < 4; i++)
	{
		j = 0;
		sprintf(sbuf, "%s", "0 ");
		//im.crop((3+(i)*10,0,13+(i)*10,h))////////////////////////////////////////////
		mn = 998;
		mx = -1;
		for(y = 0; y<24;y++)
		{
			has = 0;
			for(x = 0; x< 10; x++)
			{
				pcl = ((RGB*)output_buffer) + (3 + i * 10);
				pcl += x  + y * cinfo.output_width;
				if(pcl->R < 128 || pcl->G < 128 || pcl->B < 128)
				{
					has ++;
				}
				pcl -= x  + y * cinfo.output_width;
			}
			if(has !=0)
			{
				mn = mn<y?mn:y;
				mx = mx>y?mx:y;
			}
		}

		if(mx-mn > 12)
		{
			delta = mx-mn-12;
			mn += delta / 2;
			mx -= delta - delta / 2;
		}
		else
		{
			delta = 12-(mx-mn);
			mn -= delta / 2;
			mx += delta - delta / 2;
		}
		//piece = piece.crop((0,mn,10,mx))
		//result.append(piece)
		//printf("mx:%d mn:%d\n", mx, mn);
		tmp = (unsigned char*)pcl;
		for(x = 0;x<10; x++)
		{
			for(y=0;y<12;y++)
			{
				pcl = ((RGB*)tmp) + x + (mn+y)*cinfo.output_width;
				if(pcl->R < 128 || pcl->G < 128 || pcl->B < 128)
				{
					sprintf(sbuf, "%s%d:%.3f ", sbuf, j+1, 0.999);
				}
				else
				{
					sprintf(sbuf, "%s%d:%.3f ", sbuf, j+1, 0.001);
				}
				j++;
			}
		}
		pcl = (RGB*)tmp;
		predict(sbuf, &str[i]);
	}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        free(output_buffer);
        (void) jpeg_finish_decompress(&cinfo);

        jpeg_destroy_decompress(&cinfo);
        /* Close files, if we opened them */
        fclose(input_file);
        return 0;
}

void exit_input_error(int line_num)
{
	fprintf(stderr,"Wrong input format at line %d\n", line_num);
	exit(1);
}

void predict(char *input, char *output)
{
	int correct = 0;
	int total = 0;
	double error = 0;
	double sump = 0, sumt = 0, sumpp = 0, sumtt = 0, sumpt = 0;

	int svm_type=svm_get_svm_type(model);
	int nr_class=svm_get_nr_class(model);
	double *prob_estimates=NULL;
	int j;

	max_line_len = 1024;
	//line = (char *)malloc(max_line_len*sizeof(char));
	int i = 0;
	double target_label, predict_label;
	char *idx, *val, *label, *endptr;
	int inst_max_index = -1; // strtol gives 0 if wrong format, and precomputed kernel has <index> start from 0
	label = strtok(input," \t\n");
	if(label == NULL) // empty line
		exit_input_error(total+1);

	target_label = strtod(label,&endptr);
	if(endptr == label || *endptr != '\0')
		exit_input_error(total+1);
	while(1)
	{
		if(i>=max_nr_attr-1)	// need one more for index = -1
		{
			max_nr_attr *= 2;
			x = (struct svm_node *) realloc(x,max_nr_attr*sizeof(struct svm_node));
		}

		idx = strtok(NULL,":");
		val = strtok(NULL," \t");

		if(val == NULL)
			break;
		errno = 0;
		x[i].index = (int) strtol(idx,&endptr,10);
		if(endptr == idx || errno != 0 || *endptr != '\0' || x[i].index <= inst_max_index)
			exit_input_error(total+1);
		else
			inst_max_index = x[i].index;

		errno = 0;
		x[i].value = strtod(val,&endptr);
		if(endptr == val || errno != 0 || (*endptr != '\0' && !isspace(*endptr)))
			exit_input_error(total+1);
			++i;
	}
	x[i].index = -1;
	predict_label = svm_predict(model,x);
	//printf("%d\n", (int)predict_label);
	*output = charset[(int)predict_label];
	//printf("%p:%c:%s\n", output, *output, output);

	if(predict_label == target_label)
		++correct;
	error += (predict_label-target_label)*(predict_label-target_label);
	sump += predict_label;
	sumt += target_label;
	sumpp += predict_label*predict_label;
	sumtt += target_label*target_label;
	sumpt += predict_label*target_label;
	++total;
}

int ImgProcess(char* mod, char * img, char* str)
{
	FILE *input, *output;
	int i;
	info = &print_null;

	if((model=svm_load_model(mod))==0)
	{
		fprintf(stderr,"can't open model file %s\n", mod);
		exit(1);
	}

	x = (struct svm_node *) malloc(max_nr_attr*sizeof(struct svm_node));
	if(svm_check_probability_model(model)!=0)
		info("Model supports probability estimates, but disabled in prediction.\n");

	process_jpeg_file(img, str);
	svm_free_and_destroy_model(&model);
	free(x);
	//free(line);
	return 0;
}

int main(int argc, char* argv[])
{
	char str[5] = {0};
	//printf("%p\n", str);
	ImgProcess(argv[1], NULL, str);
	printf("%s\n", str);
	return 0;
}
