/*

                            ******************************************************

                                            CS50x Final Project

        Encrypnography (Encryption & Steganography): Hiding and Extraction of Encrypted Data into/from Images(24-bit BMP)

        By:                                      Divyansh Gaba
                                                Karandeep Singh

                            ******************************************************
*/

// Header files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include "bmp.h"
#include <stdbool.h>
#include <math.h>


//function prototypes

//function that checks if memory is Big Endian or Little Endian
bool endian();

//name explains it all
int ConvertBinaryToDecimal(long long n);

// again
bool BitsToWord(int *bits,FILE *store);

//Extracts bits from cryptic image and converts it into words again
bool decode();

//Converts text file into bits and hide it inside Image file
bool encode();

//de-encrypts data from cryptic image, called by decode
void decrypto(char *mess, char *infile, char finalfile[50]);


//Resizes images according to size of TextFile, called by encode
char* resize(char *infile,int txtSize);

//Calculates factor 'n' to resize image with, to store all the information, called by resize
int sizefactor(int txtSize,int imgSize);

//encrypts initial data with keys, called by encode
char* crypto(char inptr[50],char text[50]);

//actual encryption , called by crypto
void destroy(char word[65],char scramble[64], char hide[3][65], int select);

//actual decryption, called by decrypto
void make(int select,char hide[3][65], char string[64],char original[64]);



int main(void)
{

    int choice;
    do{

        printf("\n1. Hide\n");
        printf("2. Read\n");
        printf("3. Exit\n");
        printf("Choice: ");
        scanf("%d",&choice);
        switch(choice)
        {
            case 1:
                if(encode())
                {
                    printf("All done boys");
                }
                else
                {
                    printf("Didn't work. Maybe Try again?");
                }
                break;
            case 2:
                if(decode())
                {
                    printf("Phew All done. Wasn't Bad eh?");
                }
                else
                {
                    printf("That's some junk right there");
                }
                break;
            case 3:
                printf("kthxbye\n");
                exit(0);
            default:
                printf("Invalid Choice. Try again");
                break;
        }
        getchar();
    }while(choice!=3);
    return 0;
}


bool endian()
{
   unsigned int i = 1;
   char *c = (char*)&i;
   if (*c)
       return true;
   return false;
}


int sizefactor(int txtSize,int imgSize)
{
    int n=1;
    while(txtSize*7 >=imgSize)
    {
        imgSize*=2;
        n++;
    }
    return n;
}


int ConvertBinaryToDecimal(long long n)
{
    int decimalNumber = 0, i = 0, remainder;
    while (n!=0)
    {
        remainder = n%10;
        n /= 10;
        decimalNumber += remainder*pow(2,i);
        ++i;
    }
    return decimalNumber;
}


bool BitsToWord(int *bits,FILE *store)
{

    char c[8]; //7+NULL
    if(endian())
    {
        sprintf(c,"%d%d%d%d%d%d%d",bits[6],bits[5],bits[4],bits[3],bits[2],bits[1],bits[0]);
    }
    else
    {
        sprintf(c,"%d%d%d%d%d%d%d",bits[0],bits[1],bits[2],bits[3],bits[4],bits[5],bits[6]);
    }


    int d = atoi(c);
    char ch = ConvertBinaryToDecimal(d);
    fputc(ch,store);
    if(ch=='~')
    {
        return false;
    }

    return true;

}


char* resize(char *infile,int txtSize)
{
    int i=0;
    FILE* inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        printf("Could not open %s.\n", infile);
        return false;
    }
    FILE* outptr = fopen("resized.bmp", "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create temp file.\n");
        return false;
    }
    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {
        fclose(outptr);
        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return false;
    }

    int n=sizefactor(txtSize,bf.bfSize);
    printf("Size factor is %d\n",n);
    // determine padding for scanlines
    int inipadding =  (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    //storing width of infile
    int oldWidth = bi.biWidth;
    //storing width of outfile
    int oldHeight = abs(bi.biHeight);
    //updating dimensions for outfile
    bi.biWidth*= n;
    bi.biHeight*= n;
    //Padding for outfile
    int finpadding = (4- (bi.biWidth * sizeof(RGBTRIPLE)) % 4 ) %4;
    //updating size of outfile image (Image size is sum of all RGB triplets)
    bi.biSizeImage = abs(bi.biHeight) * (bi.biWidth * sizeof(RGBTRIPLE) + finpadding);
    //updating size of outfile, ( file size is sum of all RGB triplets  and File headers)
    bf.bfSize = bi.biSizeImage + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    // write outfile's BITMAPFILEHEADER
    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);

    // write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);




    // iterate over infile's scanlines
    for (i = 0; i < oldHeight; i++)
    {   //for increase in height in outfile
        for(int x = 0; x < n; x++)
        {
            // As we are copying the RGB triple n times into outfile, we need to read same pixel from infile n times. hence
            // we move to same position to read n times, since RGB are stored in triple hence width is multiplied by 3
            fseek(inptr, ((sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)) +(oldWidth * 3 + inipadding) * i ), SEEK_SET);
            for (int j = 0; j < oldWidth; j++)
            {
                // temporary storage
                RGBTRIPLE triple;
                // read RGB triple from infile
                fread(&triple, sizeof(RGBTRIPLE), 1, inptr);
                // for increase in width of outfile
                for(int k=0;k<n;k++)
                {
                    // write RGB triple to outfile
                    fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);
                }

            }
            // adding padding over to the new file, if needed
            for (int k = 0; k < finpadding; k++)
            {
            fputc(0x00, outptr);
            }
        }

    }




    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);

    // that's all folks
    return "resized.bmp";
}



bool encode()
{
    char infile[50],outfile[50];

    printf("Enter clean image name here: ");
    scanf("%s",infile);

    FILE* inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        printf("Could not open %s.\n", infile);
        return false;
    }
    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
            bi.biBitCount != 24 || bi.biCompression != 0)
    {

        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return false;
    }

    printf("Enter cryptic image name here: ");
    scanf("%s",outfile);

    // open output file
    FILE* outptr = fopen(outfile, "w");
    if (outptr == NULL)
    {
        fclose(inptr);
        fprintf(stderr, "Could not create %s.\n", outfile);
        return false;
    }
    char text[50];
    printf("Enter filename to hide in %s: ",infile);
    scanf("%s",text);

    FILE *words=fopen(text,"r");
    if(words==NULL)
    {

        fprintf(stderr, "Could not read %s.\n", text);
        return NULL;
    }
    //finding size of TextFile
    fseek(words,0L,SEEK_END);
    int txtSize=ftell(words);
    rewind(words);




    //call function to create crytic file "hidden" using *inptr
    char *resizedFile= resize(infile,txtSize);
    char *hidden;
    hidden=crypto(resizedFile,text);

    FILE* read=fopen(hidden,"r");
    if(read==NULL)
    {
        printf("Couldn't open %s./n",hidden);
        return false;
    }


    //variable to store all of that string
    char *info=(char*)calloc(txtSize,sizeof(char));
    int wcount=0;
    while(!feof(read))
    {
        info[wcount++]=fgetc(read);
    }
    //our way to know when to stop reading
    info[wcount++]='~';
    info[wcount]='\0';

    //close text file, we got all data in "info" string now
    fclose(read);

    /*
    for(int z=0; z<600; z++)
    {
        printf("%c",info[z]);
        if(z%64==0)
            printf("\n");
    }
    printf("\n");
    */

    //printf("%s\n",info);

    int len = strlen(info);
    //array to store bits of all characters
    int *bits=(int*)calloc(txtSize*7,sizeof(int));
    int w=0;     // get the bits out of them. Words

    // Converting characters into bits
    if(endian())
    {   for(int y =0;y<len;y++)
        {
            unsigned char ch=info[y];
            for(int z =0;z<7;z++)
            {
                bits[w++]=ch%2;
                ch=ch>>1;

            }

        }
    }
    else
    {
        for(int y =0;y<len;y++)
        {
            unsigned char ch=info[y];
            for(int z =0;z<7;z++)
            {
                bits[w++]=ch%2;
                ch=ch<<1;

            }

        }
    }

    // How many were read
    //printf("%d %d %d\n",len,w,ghgh);


    // To check later if everything got copied
    len=w;
    w=0;
    int pixels=0;

    //closing orignal file
    fclose(inptr);

    //opening resizedFile
    inptr = fopen(resizedFile, "r");
    if (inptr == NULL)
    {
        printf("Problem with resizing of img");
        return false;
    }
    // read resizedFile's BITMAPFILEHEADER
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);
    // read resizedFile's BITMAPINFOHEADER
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);
    // write outfile's BITMAPFILEHEADER

    fwrite(&bf, sizeof(BITMAPFILEHEADER), 1, outptr);
    // write outfile's BITMAPINFOHEADER
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, outptr);
   int padding =  (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;

    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {
        // iterate over pixels in scanline
        for (int j = 0; j < bi.biWidth; j++)
        {
                RGBTRIPLE triple;
                        // read RGB triple from infile
                fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

                    if(pixels%151!=0) //skip the first colour bit after every 150bits interval to remember color for key selection.
                    {
                        if(w<len)
                        {

                                    triple.rgbtRed = triple.rgbtRed%2==0?triple.rgbtRed:triple.rgbtRed-1;
                                    triple.rgbtRed = triple.rgbtRed+bits[w++];
                                if(w<len)
                                {
                                    triple.rgbtBlue = triple.rgbtBlue%2==0?triple.rgbtBlue:triple.rgbtBlue-1;
                                    triple.rgbtBlue = triple.rgbtBlue+bits[w++];
                                }
                                if(w<len)
                                {
                                    triple.rgbtGreen = triple.rgbtGreen%2==0?triple.rgbtGreen:triple.rgbtGreen-1;
                                    triple.rgbtGreen = triple.rgbtGreen+bits[w++];
                                }
                        }
                    }
                    pixels++;
                    fwrite(&triple, sizeof(RGBTRIPLE), 1, outptr);

        }
            fseek(inptr, padding, SEEK_CUR);
            // adding padding over to the new file, if needed
            for (int k = 0; k < padding; k++)
            {
            fputc(0x00, outptr);
            }

    }

    // close infile
    fclose(inptr);

    // close outfile
    fclose(outptr);
    if(w<len)
    {
        printf("Said enuff. Actually too much :( image is too small for that much data\n");
    }
    // that's all folks

    remove(hidden);
    remove("resized.bmp");
    return true;

}

char* crypto(char infile[50],char text[50])
{

    char keys[3][65];

    //keys derived from: GRC's Ultra High Security Password Generator (https://www.grc.com/passwords.htm)
    strcpy(keys[0],"D95CB76299BFDD37DA4sD35B652919002BF577C1E6B2E497A3E0B80A1DF5B637");
    strcpy(keys[1],"<@(=}doI>!GhnBd!OsdZSSF*/;_E$-Ai(/$5/:+HUqk@za4]+hbM&_5K:_Vrg?P!");
    strcpy(keys[2],"+rEMo&y:,v4I(p]*14A{!yWr{^cL6YHN@yj,08*#-DrBjk8[JzbJAD;G7=y9Q0s*");



    // open input file
    FILE* cleanimg = fopen(infile, "r");
    if (cleanimg == NULL)
    {
        printf("Could not open %s.\n", infile);
        return NULL;
    }


    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, cleanimg);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, cleanimg);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
            bi.biBitCount != 24 || bi.biCompression != 0)
    {

        fclose(cleanimg);
        fprintf(stderr, "Unsupported file format.\n");
        return NULL;
    }




    FILE *words=fopen(text,"r");
    if(words==NULL)
    {
        fclose(cleanimg);
        fprintf(stderr, "Could not create %s.\n", text);
        return NULL;
    }


    FILE *cypher=fopen("black.txt","w");


    int length=0, select=-1,size=0;
    char  c=fgetc(words);
    // determine padding for scanlines
    int padding =  (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {
        // iterate over pixels in scanline
        for (int j = 0; j < bi.biWidth; j++)
        {

            RGBTRIPLE triple;
            // read RGB triple from infile
            fread(&triple, sizeof(RGBTRIPLE), 1, cleanimg);

            //65 Value is assumed, ensure proper value

            if(length%151==0) //assume after entering 64 characters==150 triplets, we need to check the 151st RBG value, including 0;
            {
                char string[65],scramble[64];

                if( (triple.rgbtRed>triple.rgbtBlue) && (triple.rgbtRed>triple.rgbtGreen) )
                {
                    // for Red highest
                    select=0;

                }
                else if( (triple.rgbtBlue>triple.rgbtRed) && (triple.rgbtBlue>triple.rgbtGreen) )
                {
                   // for Blue highest
                   select=1;

                }
                else
                {
                    // Green highest
                    select=2;

                }

                int index;
                for(index=0; index<64 && c!=EOF; index++,c=fgetc(words),size++ )
                {
                    //replace \n with ` to preserve newline after cyptography is done
                    if(c=='\n')
                    {
                        c='`';
                        string[index]=c;

                    }
                    else
                        string[index]=c;


                }
                string[index]='\0';

                //string[index]='\0';
                // make the infile size as a multiple of 64

                if( (feof(words)) && (size%64!=0) )
                {

                    while(size%64!=0)
                    {
                        string[index]='\0';
                        size++;
                        index++;
                    }
                }

                destroy(string,scramble,keys,select);



                fwrite(scramble,sizeof(scramble), 1, cypher);
            }

            length++;
            if(c==EOF)
            {
                break;
            }
        }

        // skip over padding, if any
        fseek(cleanimg, padding, SEEK_CUR);
        if(c==EOF)
        {
            //printf("%d\n",z++);
            break;
        }

    }

    fclose(cleanimg);
    fclose(words);
    fclose(cypher);


    return "black.txt"; //returns name of cyptic file made

}

void destroy(char word[65],char scramble[64], char hide[3][65], int select)
{
    int i,upper_b=126,lower_b=32,rotate;

    rotate=upper_b-lower_b;

    for(i=0; i <64; i++)
    {
        //frequency adder
        scramble[i]=( ( (word[i]-lower_b) + (hide[select][i]-lower_b) )%rotate + lower_b);

    }
    scramble[i]='\0';




}

bool decode()
{

    char infile[50];
    printf("Enter cryptic image name here: ");
    scanf("%s",infile);

    // open input file
    FILE* inptr = fopen(infile, "r");
    if (inptr == NULL)
    {
        printf("Could not open %s.\n", infile);
        return false;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
        bi.biBitCount != 24 || bi.biCompression != 0)
    {

        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return false;
    }

    char finalfile[50];
    printf("Enter filname for decoded message: ");
    scanf("%s",finalfile);


    int padding =  (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    int bits[8], w=0, pixels=0, flag=0;
    char mess[50];

    strcpy(mess,"temp");


    FILE *store=fopen(mess,"w");


    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {
        // iterate over pixels in scanline
        for (int j = 0; j < bi.biWidth; j++)
        {
            // temporary storage
            RGBTRIPLE triple;

            // read RGB triple from infile
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);


            if( (pixels%151)!=0 )
            {
                if(flag==1) // ~ was reached
                {
                    break;
                }
                //extracting bits
                bits[w++] = triple.rgbtRed%2;
                if(w==7)
                {
                    w=0;
                   if(!BitsToWord(bits,store))
                   {
                       flag=1;
                   }
                }
                bits[w++] = triple.rgbtBlue%2;
                if(w==7)
                {
                    w=0;
                    if(!BitsToWord(bits,store))
                    {
                        flag=1;
                    }

                }
                bits[w++] = triple.rgbtGreen%2;
                if(w==7)
                {
                    w=0;
                   if(!BitsToWord(bits,store))
                   {
                       flag=1;
                   }
                }
            }
            pixels++;
        }

        if(flag==1) // ~ was reached
        {
            break;
        }
        fseek(inptr, padding, SEEK_CUR);

    }

    fclose(inptr);
    fclose(store);


    decrypto(mess,infile,finalfile);

    remove(mess);
    return true;

}
void decrypto(char *mess, char *infile, char finalfile[50])
{
    char keys[3][65];

    //keys derived from: GRC's Ultra High Security Password Generator (https://www.grc.com/passwords.htm)
    strcpy(keys[0],"D95CB76299BFDD37DA4sD35B652919002BF577C1E6B2E497A3E0B80A1DF5B637");
    strcpy(keys[1],"<@(=}doI>!GhnBd!OsdZSSF*/;_E$-Ai(/$5/:+HUqk@za4]+hbM&_5K:_Vrg?P!");
    strcpy(keys[2],"+rEMo&y:,v4I(p]*14A{!yWr{^cL6YHN@yj,08*#-DrBjk8[JzbJAD;G7=y9Q0s*");

    FILE* cryptic=fopen(mess,"r");

    FILE* out=fopen(finalfile,"w");
    if (out == NULL)
    {
        printf("Could not open %s.\n",finalfile);
        return ;
    }

    FILE* inptr=fopen(infile,"r");
    if (infile == NULL)
    {
        printf("Could not open %s.\n", infile);
        return ;
    }

    // read infile's BITMAPFILEHEADER
    BITMAPFILEHEADER bf;
    fread(&bf, sizeof(BITMAPFILEHEADER), 1, inptr);

    // read infile's BITMAPINFOHEADER
    BITMAPINFOHEADER bi;
    fread(&bi, sizeof(BITMAPINFOHEADER), 1, inptr);

    // ensure infile is (likely) a 24-bit uncompressed BMP 4.0
    if (bf.bfType != 0x4d42 || bf.bfOffBits != 54 || bi.biSize != 40 ||
            bi.biBitCount != 24 || bi.biCompression != 0)
    {

        fclose(inptr);
        fprintf(stderr, "Unsupported file format.\n");
        return ;
    }

    int pixels=0, select=-1, flag=0;
    char c=fgetc(cryptic);

    int padding =  (4 - (bi.biWidth * sizeof(RGBTRIPLE)) % 4) % 4;
    // iterate over infile's scanlines
    for (int i = 0, biHeight = abs(bi.biHeight); i < biHeight; i++)
    {
        // iterate over pixels in scanline
        for (int j = 0; j < bi.biWidth; j++)
        {

            RGBTRIPLE triple;
            // read RGB triple from infile
            fread(&triple, sizeof(RGBTRIPLE), 1, inptr);

            if(pixels%151==0) //assume after entering 64 characters==150 triplets, we need to check the 151st RBG value, including 0;
            {
                char shuffel[64],original[64];
                int index;



                if( (triple.rgbtRed>triple.rgbtBlue) && (triple.rgbtRed>triple.rgbtGreen) )
                {
                    // for Red highest
                    select=0;

                }
                else if( (triple.rgbtBlue>triple.rgbtRed) && (triple.rgbtBlue>triple.rgbtGreen) )
                {
                   // for Blue highest
                   select=1;

                }
                else
                {
                    // Green highest
                    select=2;

                }

                for(index=0; (index<64) && (c!=EOF); c=fgetc(cryptic),index++)
                {
                    if(c=='~')
                    {
                        flag=1;
                        break;
                    }
                    shuffel[index]=c;

                }

                make(select,keys,shuffel,original);

                fwrite(original, sizeof(original), 1, out );

            }
            pixels++;

        }

        if(flag==1)
        {
            break;
        }
        // skip over padding, if any
        fseek(inptr, padding, SEEK_CUR);

    }

    fclose(inptr);
    fclose(out);
    fclose(cryptic);

    //remove(mess) //->cryptic file


}
void make(int select,char hide[3][65], char string[64],char original[64])
{
    int i,upper_b=126,lower_b=32,rotate;
    rotate=upper_b-lower_b;

    for(i=0; i<64; i++)
    {

            //frequency subtractor
            original[i]= ( (string[i]-lower_b) - (hide[select][i]-lower_b) );

            //fix overflow if any
            if(original[i]<0)
            {
                 original[i]= ( (string[i]-lower_b) + rotate - (hide[select][i]-lower_b) );
            }

            //make ASCII again
            original[i]+= lower_b;

            //revert ` to \n
            if(original[i]=='`')
            {
                original[i]='\n';
            }

    }

}

