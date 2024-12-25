
#define FOR(i,n) for (i = 0;i < n;++i)
#define sv static void

#ifdef _MSC_VER
typedef unsigned __int64 ulong64;
typedef __int64 long64;
#else
typedef unsigned long long ulong64;
typedef long long long64;
#endif
typedef unsigned char u8;
typedef unsigned int ulong32;
typedef ulong32 u32;
typedef ulong64 u64;
typedef long64 i64;
typedef i64 gf[16];

static const u8
    nine[32] = {9};
static const gf
    gf0,
    gf1 = {1},
    gf121665 = {0xDB41,1},
    D = {0x78a3, 0x1359, 0x4dca, 0x75eb, 0xd8ab, 0x4141, 0x0a4d, 0x0070, 0xe898, 0x7779, 0x4079, 0x8cc7, 0xfe73, 0x2b6f, 0x6cee, 0x5203},
    D2 = {0xf159, 0x26b2, 0x9b94, 0xebd6, 0xb156, 0x8283, 0x149a, 0x00e0, 0xd130, 0xeef3, 0x80f2, 0x198e, 0xfce7, 0x56df, 0xd9dc, 0x2406},
    X = {0xd51a, 0x8f25, 0x2d60, 0xc956, 0xa7b2, 0x9525, 0xc760, 0x692c, 0xdc5c, 0xfdd6, 0xe231, 0xc0a4, 0x53fe, 0xcd6e, 0x36d3, 0x2169},
    Y = {0x6658, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666, 0x6666},
    I = {0xa0b0, 0x4a0e, 0x1b27, 0xc4ee, 0xe478, 0xad2f, 0x1806, 0x2f43, 0xd7a7, 0x3dfb, 0x0099, 0x2b4d, 0xdf0b, 0x4fc1, 0x2480, 0x2b83};

/** leave **/
sv set25519(gf r, const gf a)
{
    int i;
    FOR(i,16) r[i]=a[i];
}

/** leave **/
sv car25519(gf o)
{
    int i;
    i64 c;
    FOR(i,16) {
        o[i]+=(1LL<<16);
        c=o[i]>>16;
        o[(i+1)*(i<15)]+=c-1+37*(c-1)*(i==15);
        o[i]-=c<<16;
    }
}

/** leave **/
sv sel25519(gf p,gf q,int b)
{
    i64 t,i,c=~(b-1);
    FOR(i,16) {
        t= c&(p[i]^q[i]);
        p[i]^=t;
        q[i]^=t;
    }
}

/** leave **/
sv pack25519(u8 *o,const gf n)
{
    int i,j,b;
    gf m,t;
    FOR(i,16) t[i]=n[i];
    car25519(t);
    car25519(t);
    car25519(t);
    FOR(j,2) {
        m[0]=t[0]-0xffed;
        for(i=1;i<15;i++) {
            m[i]=t[i]-0xffff-((m[i-1]>>16)&1);
            m[i-1]&=0xffff;
        }
        m[15]=t[15]-0x7fff-((m[14]>>16)&1);
        b=(m[15]>>16)&1;
        m[14]&=0xffff;
        sel25519(t,m,1-b);
    }
    FOR(i,16) {
        o[2*i]=t[i]&0xff;
        o[2*i+1]=t[i]>>8;
    }
}

/** leave **/
static u8 par25519(const gf a)
{
    u8 d[32];
    pack25519(d,a);
    return d[0]&1;
}

/** leave **/
sv unpack25519(gf o, const u8 *n)
{
    int i;
    FOR(i,16) o[i]=n[2*i]+((i64)n[2*i+1]<<8);
    o[15]&=0x7fff;
}

/** leave **/
sv A(gf o,const gf a,const gf b)
{
    int i;
    FOR(i,16) o[i]=a[i]+b[i];
}

/** leave **/
sv Z(gf o,const gf a,const gf b)
{
    int i;
    FOR(i,16) o[i]=a[i]-b[i];
}

/** leave **/
sv M(gf o,const gf a,const gf b)
{
    i64 i,j,t[31];
    FOR(i,31) t[i]=0;
    FOR(i,16) FOR(j,16) t[i+j]+=a[i]*b[j];
    FOR(i,15) t[i]+=38*t[i+16];
    FOR(i,16) o[i]=t[i];
    car25519(o);
    car25519(o);
}

/** leave **/
sv S(gf o,const gf a)
{
    M(o,a,a);
}

/** leave **/
sv inv25519(gf o,const gf i)
{
    gf c;
    int a;
    FOR(a,16) c[a]=i[a];
    for(a=253;a>=0;a--) {
        S(c,c);
        if(a!=2&&a!=4) M(c,c,i);
    }
    FOR(a,16) o[a]=c[a];
}

/** leave **/
sv add(gf p[4],gf q[4])
{
    gf a,b,c,d,t,e,f,g,h;

    Z(a, p[1], p[0]);
    Z(t, q[1], q[0]);
    M(a, a, t);
    A(b, p[0], p[1]);
    A(t, q[0], q[1]);
    M(b, b, t);
    M(c, p[3], q[3]);
    M(c, c, D2);
    M(d, p[2], q[2]);
    A(d, d, d);
    Z(e, b, a);
    Z(f, d, c);
    A(g, d, c);
    A(h, b, a);

    M(p[0], e, f);
    M(p[1], h, g);
    M(p[2], g, f);
    M(p[3], e, h);
}

/** leave **/
sv cswap(gf p[4],gf q[4],u8 b)
{
    int i;
    FOR(i,4)
        sel25519(p[i],q[i],b);
}

/** leave **/
sv pack(u8 *r,gf p[4])
{
    gf tx, ty, zi;
    inv25519(zi, p[2]);
    M(tx, p[0], zi);
    M(ty, p[1], zi);
    pack25519(r, ty);
    r[31] ^= par25519(tx) << 7;
}

/** leave **/
sv scalarmult(gf p[4],gf q[4],const u8 *s)
{
    int i;
    set25519(p[0],gf0);
    set25519(p[1],gf1);
    set25519(p[2],gf1);
    set25519(p[3],gf0);
    for (i = 255;i >= 0;--i) {
        u8 b = (s[i/8]>>(i&7))&1;
        cswap(p,q,b);
        add(q,p);
        add(p,p);
        cswap(p,q,b);
    }
}

/** leave **/
sv scalarbase(gf p[4],const u8 *s)
{
    gf q[4];
    set25519(q[0],X);
    set25519(q[1],Y);
    set25519(q[2],gf1);
    M(q[3],X,Y);
    scalarmult(p,q,s);
}

void tweetnacl_crypto_sk_to_pk(u8 *pk, const u8 *sk)
{
    gf p[4];
    scalarbase(p,sk);
    pack(pk,p);
}

static const u64 L[32] = {0xed, 0xd3, 0xf5, 0x5c, 0x1a, 0x63, 0x12, 0x58, 0xd6, 0x9c, 0xf7, 0xa2, 0xde, 0xf9, 0xde, 0x14, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x10};

sv modL(u8 *r,i64 x[64])
{
    i64 carry,i,j;
    for (i = 63;i >= 32;--i) {
        carry = 0;
        for (j = i - 32;j < i - 12;++j) {
            x[j] += carry - 16 * x[i] * L[j - (i - 32)];
            carry = (x[j] + 128) >> 8;
            x[j] -= carry << 8;
        }
        x[j] += carry;
        x[i] = 0;
    }
    carry = 0;
    FOR(j,32) {
        x[j] += carry - (x[31] >> 4) * L[j];
        carry = x[j] >> 8;
        x[j] &= 255;
    }
    FOR(j,32) x[j] -= carry * L[j];
    FOR(i,32) {
        x[i+1] += x[i] >> 8;
        r[i] = x[i] & 255;
    }
}

sv reduce(u8 *r)
{
    i64 x[64],i;
    FOR(i,64) x[i] = (u64) r[i];
    FOR(i,64) r[i] = 0;
    modL(r,x);
}
