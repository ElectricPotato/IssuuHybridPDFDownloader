# IssuuHybridPDFDownloader
Most downloaders for Issuu.com on the web download JPGs and convert them to PDF.
This one downloads PDF pages with text you can copy and paste.
This is most usefull for music sheets and searching documents.

Based on an early version by eqagunn
http://eqagunn.blogspot.com/2012/03/issuu-publication-downloader.html
Which he has since developed thurther: https://github.com/zgarnog/issuu-pub-dl

Video tutorial:
https://www.youtube.com/watch?v=2Zdy3sQVt7U


How the downloader works:
It goes to the URL http://issuu.com/oembed?url=%DocUrl%&format=json , where %DocUrl% is replaced with the document you want to view.

For example, if the URL was https://issuu.com/scoresondemand/docs/internet_symphony_no1_eroica_37066
Then it would go to http://issuu.com/oembed?url=https://issuu.com/scoresondemand/docs/internet_symphony_no1_eroica_37066&format=json

This will give you a .json file with two important tags: "thumbnail_url" and "title"
From thumbnail_url you can get the document ID (Issuu's internal id number for each publication)
"https://image.issuu.com/130828182004-24086850515a222d1ec4d2c271cb7130/jpg/page_1_thumb_large.jpg"
Is shortened to:
"130828182004-24086850515a222d1ec4d2c271cb7130"

Now, each page of the document is stored at the URL http://page-pdf.issuu.com/%DocID%/%pageNo%.pdf ,
where %DocID% is replaced with the Document ID and %pageNo% is replaced with the page number.
For example the first page of this document is at:
http://page-pdf.issuu.com/130828182004-24086850515a222d1ec4d2c271cb7130/1.pdf

Sometimes the PDF page is missing for whatever reason, so the downloader falls back to the JPG page,
which is found at: http://image.issuu.com/%DocID%/jpg/page_%pageNo%.jpg .
The JPG is converted to a PDF page (with jpeg2pdf.exe in this case)

Finally, the pages are stitched together (here with the PDFtk utility) to form the final document.

This utility could be ported to linux if a suitible alternative for jpeg2pdf and PDFtk was found.

Using C proboably wasnt the best idea for this
