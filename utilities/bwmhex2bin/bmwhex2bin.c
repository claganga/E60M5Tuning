/*
 ============================================================================
 Name        : bmwhex2bin.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory.h>


int processRecordLine(char * pLine,size_t lineSize,FILE *output,unsigned int *nAddress){

	char buffer[64];
	int  lineAddress=0;
	int	 recordType=0;
	int	 data;

	int nByteCount;
	if(lineSize>=8){

		memcpy(buffer,pLine,2);
		buffer[2]=0x00;
		nByteCount=strtol(buffer,NULL,16);
		lineSize-=2;
		pLine+=2;

		memcpy(buffer,pLine,4);
		buffer[4]=0x00;
		lineAddress=strtol(buffer,NULL,16);
		lineSize-=4;
		pLine+=4;


		memcpy(buffer,pLine,2);
		buffer[2]=0x00;
		recordType=strtol(buffer,NULL,16);
		lineSize-=2;
		pLine+=2;

		switch(recordType){
		case 16:
		case 00:

			fseek(output,(*nAddress)+lineAddress,SEEK_SET);
			if((lineSize-2)>=(nByteCount*2)){

				while(nByteCount--){
					buffer[0]=*pLine++;
					buffer[1]=*pLine++;
					buffer[2]=0;
					data=strtol(buffer,NULL,16);
					fwrite(&data,1,1,output);
				}
			}
			break;
		case 01:
			return 1;
			break;
		case 02:
			break;
		case 03:
			break;
		case 04:
			if((lineSize-2)>=(2*2)){
				buffer[0]=*pLine++;
				buffer[1]=*pLine++;
				buffer[2]=*pLine++;
				buffer[3]=*pLine++;
				buffer[4]=0;
				*nAddress=strtol(buffer,NULL,16)<<16;
			}

			break;
		case 05:
			break;




		}


	}

	return 0;
}



int main(int argc,char *aa[]) {

	FILE *fInput=0;
	FILE *fOutput=0;
	int nReturn=-1;
	size_t nRead,nBufferSize = 256;
	char *pDataBuffer=0x00;
	char *pDataPtr;
	int nLineNumber=0;
	unsigned int nAddress=0;
	int	done=0;

	if(argc<3){
		printf("Usage : ./bmwhex2bin <in> <out> [fillk]\n");
		goto exit;
	}




	fInput = fopen (aa[1],"r");
	if(fInput==NULL){
		printf("file not found.\n");
		goto exit;
	}

	fOutput = fopen (aa[2],"w+");
	if(fOutput==NULL){
	     printf("can't create output.\n");
	     goto exit;
	}

	pDataBuffer = (char *) malloc (nBufferSize);
	if(argc==4){
		int fillSize=atol(aa[3])*1024;
		memset(pDataBuffer,0xff,nBufferSize);
		while(fillSize){
			int nWrite=(fillSize>nBufferSize)?nBufferSize:fillSize;
			fwrite(pDataBuffer,1,nWrite,fOutput);
			fillSize-=nWrite;
		}
		fseek(fOutput,0,SEEK_SET);

	}

	pDataBuffer = (char *) malloc (nBufferSize);
	memset(pDataBuffer,0x00,nBufferSize);
	while(!done && (nRead=getline(&pDataBuffer,&nBufferSize,fInput))!=-1){
		nLineNumber++;
		pDataPtr=pDataBuffer;
		switch(*pDataPtr++){
		case ':':
			if(processRecordLine(pDataPtr,nRead-1,fOutput,&nAddress)==1)
				done=1;
			break;
		case '$':
			printf("line %d $ %s",nLineNumber,pDataPtr);
			break;
		case ';':

			break;
		default:
			printf("line %d unknown prefix\n",nLineNumber);
			break;
		}
		memset(pDataBuffer,0x00,nBufferSize);
	}




exit:
	if(pDataBuffer)
		free(pDataBuffer);
	if(fInput)
		fclose(fInput);
	if(fOutput)
		fclose(fOutput);

	return nReturn;
}
