#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h> //for system() call
#include <string.h> //for strcpy(dst,src)

//cstring doesnt work with visual studio. string.h instead
#include <string.h> //for strncpy_s

void systemf(const char *template, ...) {
	va_list ap;
	va_start(ap, template);

	char buffer[1000];
	int cx;
	cx = vsnprintf(buffer, sizeof(buffer), template, ap);
	if (cx>sizeof(buffer)) {
		printf("buffer not large enough to evaluate expression %s\n", template);
		printf("evaluated length:%d, buffer:%d\n", cx, sizeof(buffer));
	}

	va_end(ap);

	//printf("system(%s);\n", buffer);//debug
	system(buffer);
}

const char* stringf(const char *template, ...) {
	va_list ap;
	va_start(ap, template);

	static char buffer[1000];
	int cx;
	cx = vsnprintf(buffer, sizeof(buffer), template, ap);
	if (cx>sizeof(buffer)) {
		printf("buffer not large enough to evaluate expression %s\n", template);
		printf("evaluated length:%d, buffer:%d\n", cx, sizeof(buffer));
	}

	va_end(ap);

	return buffer;
}

int validFile(const char *fPath) {
	FILE *fptr;
	fptr = fopen(fPath, "r");

	if (fptr == NULL) {
		return 0;
	}

	fseek(fptr, 0L, SEEK_END);
	int isEmpty = ftell(fptr) == 0; //size of file
									//fseek(fp, 0L, SEEK_SET); //You can then seek back
	fclose(fptr);

	if (isEmpty) {
		systemf("del %s", fPath); //delete the empty file
		return 0;
	}
	else {
		return 1;
	}
}

char * findTagInFile(char *fname, char *searchChars) {
	FILE *fp;

	//gcc users
	//if((fp = fopen(fname, "r")) == NULL) { return -1; }

	//Visual Studio users
	if ((fopen_s(&fp, fname, "r")) != NULL) { return -1; }

	char c;
	int pos = 0; int curSearch = 0;
	int found = 0;
	while ((c = getc(fp)) != EOF && !found) {
		if (c == searchChars[curSearch]) { // found a match
			curSearch++; //search for next char
			if (curSearch > strlen(searchChars) - 1) { // found the whole string!
				found = 1;
			}
		}
		else { // didn't find a match
			   //fseek(fp, 2-curSearch, SEEK_SET);//back to
			curSearch = 0; // go back to searching for first char 
		}
	}

	static char tagValue[256];
	//To preserve memory we use a keyword " static " which creates a single memory irrespective of the function boundary.
	//this allows it to be returned from this function
	tagValue[0] = c;
	int i = 1;

	while ((c = getc(fp)) != EOF && c != '"') {
		tagValue[i] = c;
		i++;
	}
	tagValue[i] = '\0';//add terminating char

	if (fp) { fclose(fp); }

	return tagValue;
}

int main(){
	int NPages;
	char DocURL[256];
	char DocID[46];
	char finalPath[] = ".\\downloads\\";
	char dlPath[] = ".\\downloads\\temp\\"; //stringf("%stemp\\",finalPath);
	char openPath[] = "downloads\\";//without ".\"
	char jsonFName[] = "main.json";


	printf("Issuu Publication Downloader v1.06\n");
	printf("originally by eqagunn, modified for downloading PDF by dAcid\n");
	printf("Document Url or ID >"); scanf("%s", &DocURL);

	if (strstr(DocURL, "/")) {//detect Url
		//download JSON document info file
		systemf("wget --no-check-certificate -q -O %s \"http://issuu.com/oembed?url=%s&format=json\"", jsonFName, DocURL);

		if (!validFile(jsonFName)) {
			printf("JSON file could not be downloaded at specified URL\n");
			exit(1);
		}

		char title[256];
		strcpy(title, findTagInFile(jsonFName, "\"title\":\"")); //find "title" tag in main.json

		char thumbnail_url[256];
		strcpy(thumbnail_url, findTagInFile(jsonFName, "\"thumbnail_url\":\"")); //find "thumbnail_url" tag in main.json

		//get substring from 24th char of length 45
		for (int i = 0; i < 45; i++) {
			DocID[i] = thumbnail_url[i + 24];
		}
		DocID[45] = 0;

		systemf("del %s", jsonFName); //delete the .json file

		printf("Title: %s\n", title);
		printf("Document ID: %s\n", DocID);
	} else if(strlen(DocURL)==45) {//DocID
		strcpy(DocID, DocURL);
		printf("Document ID: %s\n", DocID);
	} else {
		printf("invalid input. Expecting URL or ID of length 45 characters\n");
	}

	printf("\nNumber of Pages to download >"); scanf("%d", &NPages);

	printf("\nDownloading Pages. Please wait...\n");

	for (int page = 1; page <= NPages; page++) {
		printf("%d, ", page); fflush(stdout);

		//wget -nv -q -P ".\downloads\temp" http://page-pdf.issuu.com/%id%/%page%.pdf
		systemf("wget -q -O \"%s%d.pdf\" --timeout=900 http://page-pdf.issuu.com/%s/%d.pdf", dlPath, page, DocID, page);
		if (!validFile(stringf("%s%d.pdf", dlPath, page))) { //".\downloads\temp\%page%.pdf"
			printf("\tPage %d could not be downloaded as PDF, downloading as JPG...\n", page);

			//wget -nv -q -O ".\downloads\temp\%page%.jpg" http://image.issuu.com/%id%/jpg/page_%page%.jpg
			systemf("wget -q -O \"%s%d.jpg\" --timeout=900 http://image.issuu.com/%s/jpg/page_%d.jpg", dlPath, page, DocID, page);
			if (!validFile(stringf("%s%d.jpg", dlPath, page))) { //".\downloads\temp\%page%.jpg"
				printf("\tPage %d could not be downloaded...\n", page);
			}
			else {
				systemf("move \"%s%d.jpg\" \".\\\"");
				systemf("jpeg2pdf -o %d.pdf -p auto -n portrait -z none -r height %d.jpg", page, page);
				systemf("del %d.jpg", page);
				systemf("move \"%d.pdf\" \"s%\"", page, dlPath);
			}
		}
	}

	printf("\nStitching pages.\n");
	systemf("pdftk \"%s*.pdf\" cat output \"%sdownload.pdf\"", dlPath, finalPath);//pdftk doesnt like long output file names

	printf("\nCleaining up single page files.\n");
	systemf("del \"%s*.pdf\"", dlPath);

	printf("\nDone! Saved as \'download.pdf\' in \\downloads\\ \n");
	systemf("\"%sdownload.pdf\"", openPath);

	systemf("pause");
	return 0;
}

/*

//https://issuu.com/scoresondemand/docs/internet_symphony_no1_eroica_37066

How the downloader works:
It goes to the URL http://issuu.com/oembed?url=%DocUrl%&format=json , where %DocUrl% is replaced with the document you want to view. For example:
http://issuu.com/oembed?url=https://issuu.com/scoresondemand/docs/internet_symphony_no1_eroica_37066&format=json

This will give you a .json file with two important tags: "thumbnail_url" and "title"
From thumbnail_url you can get the document ID (Issuu's internal id number for each publication)
"https://image.issuu.com/130828182004-24086850515a222d1ec4d2c271cb7130/jpg/page_1_thumb_large.jpg"
Is shortened to:
"130828182004-24086850515a222d1ec4d2c271cb7130"

Now, each page of the document is stored at the URL http://page-pdf.issuu.com/%DocID%/%pageNo%.pdf , where %DocID% is replaced with the Document ID and %pageNo% is replaced with the page number.
For example the first page of this document is at:
http://page-pdf.issuu.com/130828182004-24086850515a222d1ec4d2c271cb7130/1.pdf

Sometimes the PDF page is missing for whatever reason, so the downloader falls back to the JPG page, which is found at: http://image.issuu.com/%DocID%/jpg/page_%pageNo%.jpg . The JPG is converted to a PDF page (with jpeg2pdf.exe in this case)

Finally, the pages are stitched together (here with the PDFtk utility) to form the final document.
*/