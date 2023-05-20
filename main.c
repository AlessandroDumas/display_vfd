
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define DIMX 1024
#define DIMY 256

#define LCX 1000 //lunghezza corde
#define DCY 48  //distanza corde
#define NC 4 //numero corde

#define DIMX7 4*9*1
#define DIMY7 7*9*1

//modi colore
#define MSOMMA ((int32_t)(0x00<<24))  //con segno !
#define MCOPRI ((int32_t)(0x01<<24))  //copre, sostituisce
#define MMEDIA ((int32_t)(0x02<<24))  //media
#define MBLEND ((int32_t)(0x03<<24))  //blend // 2nd nibble =peso [0 7] (alpha)

uint8_t frame[DIMY][DIMX];

uint8_t hex_7seg[]=
{
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111, // 9
    0b01110111, // A
    0b01111100, // b
    0b00111001, // C
    0b01011110, // d
    0b01111001, // E
    0b01110001, // F
    0b01110110, // H
    0b10000000  // .
};

void puntog(int x, int y, int g)
{
    uint32_t modo = g & 0x0f000000;
    uint8_t peso = ( g & (0x70000000) ) >> 28;
    int s;
    
    g = g & 0x80ffffff;

    switch(modo)
    {
        
        case MSOMMA:
            //somma con segno & satura
            s=(int)frame[y][x]+g;
            if(s>255)
                s=255;
            if(s<0)
                s=0;
            frame[y][x]=s;
            break;
        
        case MCOPRI:
            //sostituisce, copre
            frame[y][x]=g;
            break;

        case MMEDIA:
            //media con segno, satura
            s=((int)frame[y][x]+g)/2;
            if(s>255)
                s=255;
            if(s<0)
                s=0;
            frame[y][x]=s;
            break;

        case MBLEND:
            //pesa con segno, satura // peso=4 => media
            s=((int)frame[y][x]*(8-peso)+g*peso)/8;
            if(s>255)
                s=255;
            if(s<0)
                s=0;
            frame[y][x]=s;
            break;

        default:
            break;
    }
    
}

void display()
{
    int x,y;
    for(y=0;y<DIMY;y++)
    {
        fwrite(frame[y],1,DIMX,stdout);
    }
}

void display2x2()
{
    int x,y;
    int p;
    uint8_t r[DIMX/2];

    for(y=0;y<DIMY;y+=2)
    {
        for(x=0;x<DIMX;x+=2)
        {
            p=frame[y][x]+frame[y][x+1]+frame[y+1][x]+frame[y+1][x+1];
            r[x/2]=p/4;
        }
        fwrite(r,1,DIMX/2,stdout);
    }
}

void corde(int c)
{
    int nc;
    int x,y;
    
    int cx0=(DIMX-LCX)/2;
    int cy0=DIMY/2-(NC-1)*DCY/2;

    // corde
    for(nc=0;nc<NC;nc++)
    {
        y=cy0+nc*DCY;
        for(x=0;x<LCX;x++)
        {
            //frame[y][cx0+x]=128;
            //puntog(cx0+x,y,128/(1+x%2));
            puntog(cx0+x,y,c);
        }
    }
}

/*
void riferimenti()
{
    int x,y;
    int n;
    float pr[]={2.5, 4.5, 6.5, 8.5, 11.5};
    
    y=DIMY/2;
    
    for(n=0;n<10;n++)
    {
        x=cx0+1.0*LCX*(1-pow(2,-n/5-pr[n%5]/12.0-FRETLESS/24.0));
        if(n==4 | n==9)
        {
            simbolo(S_RIF,x,y-DCY,255);
            simbolo(S_RIF,x,y+DCY,255);
        }
        else
        {
            simbolo(S_RIF,x,y,255);
        }
    }
}
*/

void canc()
{
    int x,y;
    for(y=0;y<DIMY;y++)
        for(x=0;x<DIMX;x++)
            frame[y][x]=0;
}

/*
int comandi()
{
    int c,t,h;
    char cmd;
    char s[128];
    int nl;

    fprintf(stderr,"\n%d. ",nl++);

    scanf("%c ",&cmd);
    switch(cmd)
    {
        case '0':
            canc();
        break;
        case '1':
            scanf("%s",s);
            gprint(16,16,s,200);
        break;
        case '2':
            coordinate();
        break;
        case 'T':
            tastiera();
        break;
        case 'C':
            corde();
        break;
        case 'R':
            riferimenti();
        break;
        case 'n':
            scanf("%d %d %d",&c,&t,&h);
            nota(c,t,h);
        break;
        case 'D':
            display();
        break;
        case 'q':
            fprintf(stderr,"Ciao\n#END\n");
        break;
        case '#':
            while(getchar()!='\n');
        break;
        default:
            fprintf(stderr,"\"%c\": Comando sconosciuto\n",cmd);
        break;
    }
    
    scanf(";\n");
    //scanf("\n");

    return cmd;
}
*/

/*
        ___________
       /           \
   / \ \___________/
  /   \                  
  |   |                   
  |   |                   
  |   |                   
  |   |                   
   \ /                    
    ...                          

    ####
   #    #
   #    #
    #### 
   #    #
   #    #
    ####
*/

// codice complicato- difficile risalire al segmento
// valuta pixel per pixel
// ... per un futuro shader glsl
void cifra(int px, int py, int col)
{
    
    int xl=4*9;
    int yl=7*9;
    
    int x,y;
    int c;
    int yh; //y resto per segmenti orizzontali
    int xh; //x resto per seg v

    int ang=xl*16/32;  //angolo
    int fph=xl/7;  //frazione punte
    int fpv=yl/11;  //frazione punte

    //int nh,nv; //indici per risalire al segmento [a,b,c,d,e,f,g]
    //int nva=0;
    //int nha=0;
    
    for(y=0;y<yl;y++)
    {
        for(x=0;x<xl;x++)
        {
            //segmenti orizzontali (divide in 7 parti, accende ogni 3, spunta)
            yh=y%(yl*3/7);
            c= ( (yh*7/yl) % 3 ) == 0;  //3 rettangoli in 7 parti [0,3,6]
            c*= ( x - abs(yh-(yl/7)/2) -fph ) > 0; //spunta punte sinistra
            c*= ( xl - x - abs(yh-(yl/7)/2) -fph ) > 0;  //punte destra
            
            //nh=y/(yl*3/7); //indice del segmento [0,1,2]
            //c*= (nh == nha); // se segmento attivo
            puntog(px+ x +(ang-ang*y/yl),py+ y,c*col);  //colora
           
            //segmenti verticali (div in 4 parti, acc ogni 3, spunta)
            xh=x%(xl*3/4);
            c= ( ( xh*4/xl ) % 3 ) == 0; //2 rettangoli in 4 parti [0,3]
            c*= ( y%(yl/2) - abs(xh-(xl/4)/2) -fpv ) > 0; //spunta punte sinistra
            c*= ( ( yl - y )%(yl/2) - abs(xh-(xl/4)/2) -fpv ) > 0; //punte sinistra
            
            //nv=2*x/(xl*3/4)+y/(yl/2);//indice del segmento [0, 1 o 2]
            //c*= (nv == nva); //se segmento attivo
            puntog(px+ x +(ang-ang*y/yl),py+ y,c*col);  //colora
        }
    }
}

void lineah(int x0,int y0,int l,int c)
{
    int x;
    for(x=0;x<l;x++)
        puntog(x0+x,y0,c);
//    display();  //animazione
}
void lineav(int x0,int y0,int l,int c)
{
    int y;
    for(y=0;y<l;y++)
        puntog(x0,y0+y,c);
//    display();  //animazione
}


//disegna le griglie metalliche e i supporti sul carattere
void retine(int x0, int y0)
{
    int xl=DIMX7;
    int yl=DIMY7;
    
    int ta; //taglio punte
    
    int xs,ys;
    int x,y;
    
    int c; //colore

    c= (int32_t)8 | MCOPRI;
    //fprintf(stderr,"%x\n",c);
    
    /*  _____  */
    /* /     \ */
    //sopra
    xs=-xl/8;
    ys=-yl/14;
    for(y=0;y<yl/14;y++)
    {
        ta=(yl/7/2/2-y);
        if(ta<0) ta=0;
        lineah(x0+ta+xs, y0+y+ys, xl+xl/4-ta*2-1, c);
    }

    //sotto
    ys=yl;
    for(y=0;y<yl/14;y++)
    {
        ta=y-yl/7/2/2;
        if(ta<0) ta=0;
        lineah(x0+ta+xs, y0+y+ys, xl+xl/4-ta*2-1, c);
    }
    
    //sinistra
    ys=0;
    for(x=0;x<xl/4/2;x++)
    {
        lineav(x0+xs+x, y0+ys, yl, c);
    }

    //destra
    ys=0;
    xs=xl;
    for(x=0;x<xl/4/2;x++)
    {
        lineav(x0+xs+x, y0+ys, yl, c);
    }

    //c=-128;
    //c=0 | MCOPRI;
    c=0 | ( MMEDIA);
    //c=0 | ( MBLEND | (0x20<<24));
    //griglia
    for(y=0;y<yl;y++)
    {
        for(x=0;x<xl;x++)
        {
            if(!((x+y)%7))
                puntog(x0+x,y0+y,c);
        }
    }
}

// disegna un carattere + slot griglie retine
// disegna i segmenti attivi di un display 7 segmenti
// seg = bitmask a,b,c,d,e,f,g,.
void sseg(int x0, int y0, int seg, int c)
{

//    int xl=4*9*1;
//    int yl=7*9*1;
    int xl=DIMX7;
    int yl=DIMY7;
    
    int ta; //taglio punte
    
    int xs,ys;
    int x,y;

    //a
    if(seg & 1<<0)
    {
        xs=0; ys=0;
        x=xl/4/2;
        for(y=0;y<yl/7;y++)
        {
            ta=abs(yl/7/2-y);
            lineah(x0+xs + x+ta ,y0+ys + y, xl/4*3-ta*2, c);
        }
    }
    //d
    if(seg & 1<<3)
    {
        xs=0; ys=yl*6/7;
        x=xl/4/2;
        for(y=0;y<yl/7;y++)
        {
            ta=abs(yl/7/2-y);
            lineah(x0+xs + x+ta ,y0+ys + y, xl/4*3-ta*2, c);
        }
    }
    //g
    if(seg & 1<<6)
    {
        xs=0; ys=yl*3/7;
        x=xl/4/2;
        for(y=0;y<yl/7;y++)
        {
            ta=abs(yl/7/2-y);
            lineah(x0+xs + x+ta ,y0+ys + y, xl/4*3-ta*2, c);
        }
    }
    
    //b
    if(seg & 1<<1)
    {
        xs=xl*3/4 -1; ys=0;
        y=yl*2/14;
        for(x=0;x<xl/4;x++)
        {
            ta=abs(xl/4/2-x);
            lineav(x0+xs + x, y0+ys + y+ta, yl*4/14-ta*2, c);
        }
    }
    //c
    if(seg & 1<<2)
    {
        xs=xl*3/4 -1; ys=0;
        y=yl*8/14;
        for(x=0;x<xl/4;x++)
        {
            ta=abs(xl/4/2-x);
            lineav(x0+xs + x, y0+ys + y+ta, yl*4/14-ta*2, c);
        }
    }
    //e
    if(seg & 1<<4)
    {
        xs=0; ys=0;
        y=yl*8/14;
        for(x=0;x<xl/4;x++)
        {
            ta=abs(xl/4/2-x);
            lineav(x0+xs + x, y0+ys + y+ta, yl*4/14-ta*2, c);
        }
    }
    //f
    if(seg & 1<<5)
    {
        xs=0; ys=0;
        y=yl*2/14;
        for(x=0;x<xl/4;x++)
        {
            ta=abs(xl/4/2-x);
            lineav(x0+xs + x, y0+ys + y+ta, yl*4/14-ta*2, c);
        }
    }
    //h (.)
    if(seg & 1<<7)
    {
        xs=xl*3/4+xl/4/2; ys=yl*13/14+1;
        x=0;
        y=0;
        for(x=0;x<xl/4;x++)
        {
            ta=abs(xl/4/2-x);
            //lineav(x0+xs + x, y0+ys + y+ta, yl*2/14-ta*2, c);
            lineav(x0+xs + x, y0+ys + y+ta, xl/4-ta*2-1, c);
        }
    }

    retine(x0,y0);
}


void sseg_demo(int x0, int y0, int c)
{
    int n;
    for(n=0;n<18;n++)
    {
        sseg(x0+ 6*9*n,y0,hex_7seg[n] | ((n%3)==0)<<7, c);
    }
}

void circuiti(int c)
{
    int x,y,z;
    int px,py,pz;
    int l;

/*
    for(y=0;y<DIMY;y++)
        for(x=0;x<DIMX;x++)
*/

    for(y=0;y<DIMY;y++)
    {
        if(((y*y*y)%17)*((y*y)%19)==0)
        {
            px=((y*y)%(DIMX*2/3)+1);
            //fprintf(stderr,"px=%d\n",px);
            l=(DIMX-px)/2;
            lineah(px,y,l,c/8);
            lineah(px,y+1,l,c/8);
            puntog(px,y,c);
            puntog(px+l,y,c);
        }
        if(((y*y)%13)*((y*y*y)%14)==0)
        {
            lineah(px,y,l,c/2);
            lineah(px,y+1,l,c/8);
            puntog(px,y,c);
            puntog(px+l,y,c);
        }
    }
}

void fondo(int c)
{
    int x,y;

    for(y=0;y<DIMY;y++)
    {
        for(x=0;x<DIMX;x++)
        {
            frame[y][x]=c*(4+3*(((3*x*x*y/5)%13)==0)+(8*x*y*y/7)%7+(8*x*y/3)%9);
        }
    }
}

int main()
{
    
    fprintf(stderr,"Dimensione quadro: %dx%d\n",DIMX,DIMY);

    //cifra(32,32,255);
    //sseg(32,128, 255, 255);
    
    canc();
    fondo(2);
    display();
    canc();
    circuiti(64);
    display();
    canc();
    sseg_demo(32,32,192);
    sseg_demo(32,128,192);
    display();
    canc();
    corde(128 | MBLEND | (6<<28) );
    
    display();
    //display2x2();
    
    /*
    int cmd;
    
    cmd=0;
    while(cmd!='q')
    {
        cmd=comandi();
    }
    */

    return 0;
}

