#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "svm-scale.h"
#include "common.h"

static void exit_with_help()//添加static，保证各个C文件的同名函数不冲突
{
	printf(
	"Usage: svm-scale [options] data_filename\n"
	"options:\n"
	"-l lower : x scaling lower limit (default -1)\n"
	"-u upper : x scaling upper limit (default +1)\n"
	"-y y_lower y_upper : y scaling limits (default: no y scaling)\n"
	"-s save_filename : save scaling parameters to save_filename\n"
	"-r restore_filename : restore scaling parameters from restore_filename\n"
	);
	exit(1);
}

static char *line = NULL;//添加static，保证各个C文件的同名函数不冲突
static int max_line_len = 1024;//添加static，保证各个C文件的同名函数不冲突
double lower=-1.0,upper=1.0,y_lower,y_upper;
int y_scaling = 0;
double *feature_max;
double *feature_min;
double y_max = -DBL_MAX;
double y_min = DBL_MAX;
int max_index;
int min_index;
long int num_nonzeros = 0;
long int new_num_nonzeros = 0;
FILE *fp_result_filename = NULL;

#define max(x,y) (((x)>(y))?(x):(y))
#define min(x,y) (((x)<(y))?(x):(y))

void output_target(double value);
void output(int index, double value);
static char* readline(FILE *input);//添加static，保证各个C文件的同名函数不冲突
int clean_up(FILE *fp_restore, FILE *fp, const char *msg);

int main_svm_scale(PARA_SVM_SCALE *para_svm_scale)
{
	int i,index;
	FILE *fp, *fp_restore = NULL;
	char *save_filename = NULL;
	char *restore_filename = NULL;

//	for(i=1;i<argc;i++)
//	{
//		if(argv[i][0] != '-') break;
//		++i;
//		switch(argv[i-1][1])
//		{
//			case 'l': lower = atof(argv[i]); break;
//			case 'u': upper = atof(argv[i]); break;
//			case 'y':
//				y_lower = atof(argv[i]);
//				++i;
//				y_upper = atof(argv[i]);
//				y_scaling = 1;
//				break;
//			case 's': save_filename = argv[i]; break;
//			case 'r': restore_filename = argv[i]; break;
//			default:
//				fprintf(stderr,"unknown option\n");
//				exit_with_help();
//		}
//	}
    lower = para_svm_scale->l;
    upper = para_svm_scale->u;
    y_scaling = para_svm_scale->y_scaling;
    if(y_scaling)
    {
        y_lower = para_svm_scale->y_lower;
        y_upper = para_svm_scale->y_upper;
    }
    save_filename = para_svm_scale->save_filename;
    restore_filename = para_svm_scale->restore_filename;


	if(!(upper > lower) || (y_scaling && !(y_upper > y_lower)))
	{
		fprintf(stderr,"inconsistent lower/upper specification\n");
        exit_with_help();
	}
	
	if(restore_filename && save_filename)
	{
		fprintf(stderr,"cannot use -r and -s simultaneously\n");
        exit_with_help();
	}

//	if(argc != i+1)
//		exit_with_help();

//	fp=fopen(argv[i],"r");

    /* 打开原始数据文件 */
    if((fp = fopen(para_svm_scale->data_set,"r")) == NULL)
	{
        fprintf(stderr,"can't open data file %s\n", para_svm_scale->data_set);
		exit(1);
	}

    /* 打开保存结果文件 */
    if((fp_result_filename = fopen(para_svm_scale->result_filename,"w")) == NULL)
    {
        fprintf(stderr,"can't open result file %s\n", para_svm_scale->result_filename);
        exit(1);
    }

	line = (char *) malloc(max_line_len*sizeof(char));

#define SKIP_TARGET\
	while(isspace(*p)) ++p;\
	while(!isspace(*p)) ++p;

#define SKIP_ELEMENT\
	while(*p!=':') ++p;\
	++p;\
	while(isspace(*p)) ++p;\
	while(*p && !isspace(*p)) ++p;
	
	/* assumption: min index of attributes is 1 */
	/* pass 1: find out max index of attributes */
	max_index = 0;
	min_index = 1;

	if(restore_filename)
	{
		int idx, c;

		fp_restore = fopen(restore_filename,"r");
		if(fp_restore==NULL)
		{
			fprintf(stderr,"can't open file %s\n", restore_filename);
			exit(1);
		}

		c = fgetc(fp_restore);
		if(c == 'y')
		{
			readline(fp_restore);
			readline(fp_restore);
			readline(fp_restore);
		}
		readline(fp_restore);
		readline(fp_restore);

		while(fscanf(fp_restore,"%d %*f %*f\n",&idx) == 1)
			max_index = max(idx,max_index);
		rewind(fp_restore);
	}

	while(readline(fp)!=NULL)
	{
		char *p=line;

		SKIP_TARGET

		while(sscanf(p,"%d:%*f",&index)==1)
		{
			max_index = max(max_index, index);
			min_index = min(min_index, index);
			SKIP_ELEMENT
			num_nonzeros++;
		}
	}

	if(min_index < 1)
		fprintf(stderr,
			"WARNING: minimal feature index is %d, but indices should start from 1\n", min_index);

	rewind(fp);

	feature_max = (double *)malloc((max_index+1)* sizeof(double));
	feature_min = (double *)malloc((max_index+1)* sizeof(double));

	if(feature_max == NULL || feature_min == NULL)
	{
		fprintf(stderr,"can't allocate enough memory\n");
		exit(1);
	}

	for(i=0;i<=max_index;i++)
	{
		feature_max[i]=-DBL_MAX;
		feature_min[i]=DBL_MAX;
	}

	/* pass 2: find out min/max value */
	while(readline(fp)!=NULL)
	{
		char *p=line;
		int next_index=1;
		double target;
		double value;

		if (sscanf(p,"%lf",&target) != 1)
            return clean_up(fp_restore, fp, "ERROR: failed to read labels\n");
		y_max = max(y_max,target);
		y_min = min(y_min,target);
		
		SKIP_TARGET

		while(sscanf(p,"%d:%lf",&index,&value)==2)
		{
			for(i=next_index;i<index;i++)
			{
				feature_max[i]=max(feature_max[i],0);
				feature_min[i]=min(feature_min[i],0);
			}
			
			feature_max[index]=max(feature_max[index],value);
			feature_min[index]=min(feature_min[index],value);

			SKIP_ELEMENT
			next_index=index+1;
		}		

		for(i=next_index;i<=max_index;i++)
		{
			feature_max[i]=max(feature_max[i],0);
			feature_min[i]=min(feature_min[i],0);
		}	
	}

	rewind(fp);

	/* pass 2.5: save/restore feature_min/feature_max */
	
	if(restore_filename)
	{
		/* fp_restore rewinded in finding max_index */
		int idx, c;
		double fmin, fmax;
		int next_index = 1;
		
		if((c = fgetc(fp_restore)) == 'y')
		{
			if(fscanf(fp_restore, "%lf %lf\n", &y_lower, &y_upper) != 2 ||
			   fscanf(fp_restore, "%lf %lf\n", &y_min, &y_max) != 2)
				return clean_up(fp_restore, fp, "ERROR: failed to read scaling parameters\n");
			y_scaling = 1;
		}
		else
			ungetc(c, fp_restore);

		if (fgetc(fp_restore) == 'x') 
		{
			if(fscanf(fp_restore, "%lf %lf\n", &lower, &upper) != 2)
				return clean_up(fp_restore, fp, "ERROR: failed to read scaling parameters\n");
			while(fscanf(fp_restore,"%d %lf %lf\n",&idx,&fmin,&fmax)==3)
			{
				for(i = next_index;i<idx;i++)
					if(feature_min[i] != feature_max[i])
						fprintf(stderr,
							"WARNING: feature index %d appeared in file %s was not seen in the scaling factor file %s.\n",
                            i, para_svm_scale->data_set, restore_filename);//argv[argc-1]--->para_svm_scale->data_set,准备缩放的数据文件中出现了某一个特征，但是缩放规则文件中却没有缩放因子。

				feature_min[idx] = fmin;
				feature_max[idx] = fmax;

				next_index = idx + 1;
			}
			
			for(i=next_index;i<=max_index;i++)
				if(feature_min[i] != feature_max[i])
					fprintf(stderr,
						"WARNING: feature index %d appeared in file %s was not seen in the scaling factor file %s.\n",
                        i, para_svm_scale->data_set, restore_filename);//argv[argc-1]--->para_svm_scale->data_set,准备缩放的数据文件中出现了某一个特征，但是缩放规则文件中却没有缩放因子。
		}
		fclose(fp_restore);
	}

	if(save_filename)
	{
		FILE *fp_save = fopen(save_filename,"w");
		if(fp_save==NULL)
		{
			fprintf(stderr,"can't open file %s\n", save_filename);
			exit(1);
		}
		if(y_scaling)
		{
			fprintf(fp_save, "y\n");
			fprintf(fp_save, "%.16g %.16g\n", y_lower, y_upper);
			fprintf(fp_save, "%.16g %.16g\n", y_min, y_max);
		}
		fprintf(fp_save, "x\n");
		fprintf(fp_save, "%.16g %.16g\n", lower, upper);
		for(i=1;i<=max_index;i++)
		{
			if(feature_min[i]!=feature_max[i])
				fprintf(fp_save,"%d %.16g %.16g\n",i,feature_min[i],feature_max[i]);
		}

		if(min_index < 1)
			fprintf(stderr,
				"WARNING: scaling factors with indices smaller than 1 are not stored to the file %s.\n", save_filename);

		fclose(fp_save);
	}
	
	/* pass 3: scale */
	while(readline(fp)!=NULL)
	{
		char *p=line;
		int next_index=1;
		double target;
		double value;
		
		if (sscanf(p,"%lf",&target) != 1)
			return clean_up(NULL, fp, "ERROR: failed to read labels\n");
		output_target(target);

		SKIP_TARGET

		while(sscanf(p,"%d:%lf",&index,&value)==2)
		{
			for(i=next_index;i<index;i++)
				output(i,0);
			
			output(index,value);

			SKIP_ELEMENT
			next_index=index+1;
		}		

		for(i=next_index;i<=max_index;i++)
			output(i,0);

        /* 保存换行符 */
        fprintf(fp_result_filename, "\n");
        //printf("\n");
	}

	if (new_num_nonzeros > num_nonzeros)
		fprintf(stderr, 
			"WARNING: original #nonzeros %ld\n"
			"         new      #nonzeros %ld\n"
			"Use -l 0 if many original feature values are zeros\n",
			num_nonzeros, new_num_nonzeros);

	free(line);
	free(feature_max);
	free(feature_min);
	fclose(fp);
    fclose(fp_result_filename);
    return SUCCESS;
}

char* readline(FILE *input)
{
	int len;
	
	if(fgets(line,max_line_len,input) == NULL)
		return NULL;

	while(strrchr(line,'\n') == NULL)
	{
		max_line_len *= 2;
		line = (char *) realloc(line, max_line_len);
		len = (int) strlen(line);
		if(fgets(line+len,max_line_len-len,input) == NULL)
			break;
	}
	return line;
}

void output_target(double value)
{
	if(y_scaling)
	{
		if(value == y_min)
			value = y_lower;
		else if(value == y_max)
			value = y_upper;
		else value = y_lower + (y_upper-y_lower) *
			     (value - y_min)/(y_max-y_min);
	}
    /* 保存类别标签 */
    fprintf(fp_result_filename,"%g ",value);
    //printf("%g ",value);
}

void output(int index, double value)
{
	/* skip single-valued attribute */
	if(feature_max[index] == feature_min[index])
		return;

	if(value == feature_min[index])
		value = lower;
	else if(value == feature_max[index])
		value = upper;
	else
		value = lower + (upper-lower) * 
			(value-feature_min[index])/
			(feature_max[index]-feature_min[index]);

	if(value != 0)
	{
        /* 保存缩放数据 */
        fprintf(fp_result_filename,"%d:%g ",index, value);
        //printf("%d:%g ",index, value);
		new_num_nonzeros++;
	}
}

int clean_up(FILE *fp_restore, FILE *fp, const char* msg)
{
	fprintf(stderr,	"%s", msg);
	free(line);
	free(feature_max);
	free(feature_min);
	fclose(fp);
	if (fp_restore)
		fclose(fp_restore);
	return -1;
}

