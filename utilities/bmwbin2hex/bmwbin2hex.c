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


int processRecordLine(char * line,size_t lineSize,FILE *binInput,FILE *output,unsigned int *nAddress){

	char buffer[256];
	char *pLine=line;
	unsigned short  lineAddress=0;
	unsigned short	recordType=0;
	unsigned short 	nByteCount=0;
	unsigned char	linecrc=0;
	int	 nByteIndex=0;

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
			fprintf(output,":%02X%04X%02X",nByteCount,lineAddress,recordType);
			linecrc+=(nByteCount>>0)&0xff;

			linecrc+=(lineAddress>>8)&0xff;
			linecrc+=(lineAddress>>0)&0xff;

			linecrc+=(recordType>>0)&0xff;

			fseek(binInput,(*nAddress)+lineAddress,SEEK_SET);
			if((lineSize-2)>=(nByteCount*2)){

				int nRead=fread(buffer,1,nByteCount,binInput);

				while(nRead>0){
					linecrc+=buffer[nByteIndex]&0xff;
					fprintf(output,"%02X",buffer[nByteIndex]&0xff);
					nByteIndex++;
					nRead--;
				}
				linecrc=(~linecrc)+1;
				fprintf(output,"%02X\r\n",linecrc&0xff);
			}
			break;
		case 01:
			fwrite(":",1,1,output);
			fwrite(line,1,strlen(line),output);
			return 1;
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
		//Fall through
		case 05:
		case 02:
		case 03:
		default:
			fwrite(":",1,1,output);
			fwrite(line,1,strlen(line),output);
		break;




		}


	}

	return 0;
}



int main(int argc,char *aa[]) {

	FILE *fInputHex=0;
	FILE *fInputBin=0;
	FILE *fOutput=0;
	int nReturn=-1;
	size_t nRead,nBufferSize = 256;
	char *pDataBuffer=0x00;
	char *pDataPtr;
	int nLineNumber=0;
	unsigned int nAddress=0;
	int	done=0;

	if(argc<3){
		printf("Usage : ./bmwbin2hex <in bin> <in hex> <out hex>\n");
		goto exit;
	}


	fInputBin = fopen (aa[1],"r");
	if(fInputBin==NULL){
		printf("file not found %s.\n",aa[1]);
		goto exit;
	}

	fInputHex = fopen (aa[2],"r");
	if(fInputHex==NULL){
		printf("file not found %s.\n",aa[1]);
		goto exit;
	}

	fOutput = fopen (aa[3],"w+");
	if(fOutput==NULL){
	     printf("can't create output.\n");
	     goto exit;
	}

	pDataBuffer = (char *) malloc (nBufferSize);
	memset(pDataBuffer,0x00,nBufferSize);
	while(!done && (nRead=getline(&pDataBuffer,&nBufferSize,fInputHex))!=-1){
		nLineNumber++;
		pDataPtr=pDataBuffer;
		switch(*pDataPtr++){
		case ':':
			if(processRecordLine(pDataPtr,nRead-1,fInputBin,fOutput,&nAddress)==1)
				done=1;
			break;
		case '$':
		case ';':
		default:
			fwrite(pDataBuffer,nRead,1,fOutput);
			break;
		}
		memset(pDataBuffer,0x00,nBufferSize);
	}




exit:
	if(pDataBuffer)
		free(pDataBuffer);
	if(fInputHex)
		fclose(fInputHex);
	if(fInputBin)
		fclose(fInputBin);
	if(fOutput)
		fclose(fOutput);

	return nReturn;
}
