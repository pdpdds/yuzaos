/***** This file contains automatically generated code ******
**
** The code in this file has been automatically generated by
**
**   sqlite/tool/mkkeywordhash.c
**
** The code in this file implements a function that determines whether
** or not a given identifier is really an SQL keyword.  The same thing
** might be implemented more directly using a hand-written hash table.
** But by using this automatically generated code, the size of the code
** is substantially reduced.  This is important for embedded applications
** on platforms with limited memory.
*/
/* Hash score: 177 */
static int keywordCode(const char *z, int n){
  /* zText[] encodes 819 bytes of keywords in 545 bytes */
  /*   REINDEXEDESCAPEACHECKEYBEFOREIGNOREGEXPLAINSTEADDATABASELECT       */
  /*   ABLEFTHENDEFERRABLELSEXCEPTRANSACTIONATURALTERAISEXCLUSIVE         */
  /*   XISTSAVEPOINTERSECTRIGGEREFERENCESCONSTRAINTOFFSETEMPORARY         */
  /*   UNIQUERYWITHOUTERELEASEATTACHAVINGROUPDATEBEGINNERENAMEBETWEEN     */
  /*   OTNULLIKECASCADELETECASECOLLATECREATECURRENT_DATEDETACH            */
  /*   IMMEDIATEJOINSERTMATCHPLANALYZEPRAGMABORTVALUESVIRTUALIMITWHEN     */
  /*   WHEREPLACEAFTERESTRICTANDEFAULTAUTOINCREMENTCASTCOLUMNCOMMIT       */
  /*   CONFLICTCROSSCURRENT_TIMESTAMPRIMARYDEFERREDISTINCTDROPFAIL        */
  /*   FROMFULLGLOBYIFISNULLORDERIGHTROLLBACKROWUNIONUSINGVACUUMVIEW      */
  /*   INITIALLY                                                          */
  static const char zText[544] = {
    'R','E','I','N','D','E','X','E','D','E','S','C','A','P','E','A','C','H',
    'E','C','K','E','Y','B','E','F','O','R','E','I','G','N','O','R','E','G',
    'E','X','P','L','A','I','N','S','T','E','A','D','D','A','T','A','B','A',
    'S','E','L','E','C','T','A','B','L','E','F','T','H','E','N','D','E','F',
    'E','R','R','A','B','L','E','L','S','E','X','C','E','P','T','R','A','N',
    'S','A','C','T','I','O','N','A','T','U','R','A','L','T','E','R','A','I',
    'S','E','X','C','L','U','S','I','V','E','X','I','S','T','S','A','V','E',
    'P','O','I','N','T','E','R','S','E','C','T','R','I','G','G','E','R','E',
    'F','E','R','E','N','C','E','S','C','O','N','S','T','R','A','I','N','T',
    'O','F','F','S','E','T','E','M','P','O','R','A','R','Y','U','N','I','Q',
    'U','E','R','Y','W','I','T','H','O','U','T','E','R','E','L','E','A','S',
    'E','A','T','T','A','C','H','A','V','I','N','G','R','O','U','P','D','A',
    'T','E','B','E','G','I','N','N','E','R','E','N','A','M','E','B','E','T',
    'W','E','E','N','O','T','N','U','L','L','I','K','E','C','A','S','C','A',
    'D','E','L','E','T','E','C','A','S','E','C','O','L','L','A','T','E','C',
    'R','E','A','T','E','C','U','R','R','E','N','T','_','D','A','T','E','D',
    'E','T','A','C','H','I','M','M','E','D','I','A','T','E','J','O','I','N',
    'S','E','R','T','M','A','T','C','H','P','L','A','N','A','L','Y','Z','E',
    'P','R','A','G','M','A','B','O','R','T','V','A','L','U','E','S','V','I',
    'R','T','U','A','L','I','M','I','T','W','H','E','N','W','H','E','R','E',
    'P','L','A','C','E','A','F','T','E','R','E','S','T','R','I','C','T','A',
    'N','D','E','F','A','U','L','T','A','U','T','O','I','N','C','R','E','M',
    'E','N','T','C','A','S','T','C','O','L','U','M','N','C','O','M','M','I',
    'T','C','O','N','F','L','I','C','T','C','R','O','S','S','C','U','R','R',
    'E','N','T','_','T','I','M','E','S','T','A','M','P','R','I','M','A','R',
    'Y','D','E','F','E','R','R','E','D','I','S','T','I','N','C','T','D','R',
    'O','P','F','A','I','L','F','R','O','M','F','U','L','L','G','L','O','B',
    'Y','I','F','I','S','N','U','L','L','O','R','D','E','R','I','G','H','T',
    'R','O','L','L','B','A','C','K','R','O','W','U','N','I','O','N','U','S',
    'I','N','G','V','A','C','U','U','M','V','I','E','W','I','N','I','T','I',
    'A','L','L','Y',
  };
  static const unsigned char aHash[127] = {
      75, 104, 115,  73,   0,  45,   0,   0,  81,   0,  76,   0,   0,
      42,  12,  77,  15,   0, 114,  84,  53, 111,   0,  19,   0,   0,
     119,   0, 117,  88,   0,  22,  92,   0,   9,   0,   0,  69,  70,
       0,  68,   6,   0,  48,  89, 101,   0, 116, 100,   0,   0,  44,
       0, 102,  24,   0,  17,   0, 120,  52,  23,   0,   5, 109,  25,
      95,   0,   0, 122, 105,  59, 121,  56,  28,  54,   0,  90,   0,
      99,  26,   0,  98,   0,   0,   0,  94,  91,  96,  87, 108,  14,
      39, 107,   0,  80,   0,  18,  86, 110,  32,   0, 118,  79, 112,
      61,  46,  83,   0,   0,  93,  40,   0, 113,   0,  36,   0,   0,
      29,   0,  85,  62,  63,   0,  20,  60,   0,  55,
  };
  static const unsigned char aNext[122] = {
       0,   0,   0,   0,   4,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   2,   0,   0,   0,   0,   0,   0,  13,   0,   0,   0,   0,
       0,   7,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
       0,   0,   0,   0,  33,   0,  21,   0,   0,   0,   0,   0,   0,
      43,   3,  47,   0,   0,   0,   0,  30,   0,  57,   0,  38,   0,
       0,   0,   1,  65,   0,   0,  66,   0,  41,   0,   0,   0,   0,
       0,   0,  49,  64,   0,   0,   0,  51,  31,   0,  16,  34,  10,
       0,   0,   0,   0,   0,   0,   0,  11,  71,  78,   0,   8,   0,
     103,  97,   0, 106,   0,  58,   0,  74,  50,  27,  37,  72,  82,
       0,  35,  67,   0,   0,
  };
  static const unsigned char aLen[122] = {
       7,   7,   5,   4,   6,   4,   5,   3,   6,   7,   3,   6,   6,
       7,   7,   3,   8,   2,   6,   5,   4,   4,   3,  10,   4,   6,
      11,   6,   2,   7,   5,   5,   9,   6,   9,   9,   7,  10,  10,
       4,   6,   2,   3,   9,   4,   2,   6,   5,   7,   5,   7,   6,
       6,   5,   6,   5,   5,   6,   7,   7,   3,   2,   4,   4,   7,
       3,   6,   4,   7,   6,  12,   6,   9,   4,   6,   5,   4,   7,
       6,   5,   6,   7,   5,   4,   5,   7,   5,   8,   3,   7,  13,
       2,   2,   4,   6,   6,   8,   5,  17,  12,   7,   8,   8,   2,
       4,   4,   4,   4,   4,   2,   2,   6,   5,   5,   8,   3,   5,
       5,   6,   4,   9,   3,
  };
  static const unsigned short int aOffset[122] = {
       0,   2,   2,   8,   9,  14,  16,  20,  23,  25,  25,  29,  33,
      36,  41,  46,  48,  53,  54,  59,  62,  65,  67,  69,  78,  81,
      86,  91,  95,  96, 101, 105, 109, 117, 122, 128, 136, 142, 152,
     159, 162, 162, 165, 167, 167, 171, 176, 179, 184, 188, 192, 199,
     204, 209, 212, 218, 221, 225, 231, 237, 237, 237, 240, 243, 247,
     248, 252, 258, 262, 269, 275, 287, 293, 302, 304, 310, 315, 317,
     324, 329, 334, 340, 346, 351, 355, 358, 365, 369, 377, 379, 386,
     388, 390, 399, 403, 409, 415, 423, 428, 428, 444, 451, 458, 459,
     466, 470, 474, 478, 482, 485, 487, 489, 495, 499, 504, 512, 515,
     520, 525, 531, 535, 540,
  };
  static const unsigned char aCode[122] = {
    TK_REINDEX,    TK_INDEXED,    TK_INDEX,      TK_DESC,       TK_ESCAPE,     
    TK_EACH,       TK_CHECK,      TK_KEY,        TK_BEFORE,     TK_FOREIGN,    
    TK_FOR,        TK_IGNORE,     TK_LIKE_KW,    TK_EXPLAIN,    TK_INSTEAD,    
    TK_ADD,        TK_DATABASE,   TK_AS,         TK_SELECT,     TK_TABLE,      
    TK_JOIN_KW,    TK_THEN,       TK_END,        TK_DEFERRABLE, TK_ELSE,       
    TK_EXCEPT,     TK_TRANSACTION,TK_ACTION,     TK_ON,         TK_JOIN_KW,    
    TK_ALTER,      TK_RAISE,      TK_EXCLUSIVE,  TK_EXISTS,     TK_SAVEPOINT,  
    TK_INTERSECT,  TK_TRIGGER,    TK_REFERENCES, TK_CONSTRAINT, TK_INTO,       
    TK_OFFSET,     TK_OF,         TK_SET,        TK_TEMP,       TK_TEMP,       
    TK_OR,         TK_UNIQUE,     TK_QUERY,      TK_WITHOUT,    TK_JOIN_KW,    
    TK_RELEASE,    TK_ATTACH,     TK_HAVING,     TK_GROUP,      TK_UPDATE,     
    TK_BEGIN,      TK_JOIN_KW,    TK_RENAME,     TK_BETWEEN,    TK_NOTNULL,    
    TK_NOT,        TK_NO,         TK_NULL,       TK_LIKE_KW,    TK_CASCADE,    
    TK_ASC,        TK_DELETE,     TK_CASE,       TK_COLLATE,    TK_CREATE,     
    TK_CTIME_KW,   TK_DETACH,     TK_IMMEDIATE,  TK_JOIN,       TK_INSERT,     
    TK_MATCH,      TK_PLAN,       TK_ANALYZE,    TK_PRAGMA,     TK_ABORT,      
    TK_VALUES,     TK_VIRTUAL,    TK_LIMIT,      TK_WHEN,       TK_WHERE,      
    TK_REPLACE,    TK_AFTER,      TK_RESTRICT,   TK_AND,        TK_DEFAULT,    
    TK_AUTOINCR,   TK_TO,         TK_IN,         TK_CAST,       TK_COLUMNKW,   
    TK_COMMIT,     TK_CONFLICT,   TK_JOIN_KW,    TK_CTIME_KW,   TK_CTIME_KW,   
    TK_PRIMARY,    TK_DEFERRED,   TK_DISTINCT,   TK_IS,         TK_DROP,       
    TK_FAIL,       TK_FROM,       TK_JOIN_KW,    TK_LIKE_KW,    TK_BY,         
    TK_IF,         TK_ISNULL,     TK_ORDER,      TK_JOIN_KW,    TK_ROLLBACK,   
    TK_ROW,        TK_UNION,      TK_USING,      TK_VACUUM,     TK_VIEW,       
    TK_INITIALLY,  TK_ALL,        
  };
  int h, i;
  if( n<2 ) return TK_ID;
  h = ((charMap(z[0])*4) ^
      (charMap(z[n-1])*3) ^
      n) % 127;
  for(i=((int)aHash[h])-1; i>=0; i=((int)aNext[i])-1){
    if( aLen[i]==n && sqlite3StrNICmp(&zText[aOffset[i]],z,n)==0 ){
      testcase( i==0 ); /* REINDEX */
      testcase( i==1 ); /* INDEXED */
      testcase( i==2 ); /* INDEX */
      testcase( i==3 ); /* DESC */
      testcase( i==4 ); /* ESCAPE */
      testcase( i==5 ); /* EACH */
      testcase( i==6 ); /* CHECK */
      testcase( i==7 ); /* KEY */
      testcase( i==8 ); /* BEFORE */
      testcase( i==9 ); /* FOREIGN */
      testcase( i==10 ); /* FOR */
      testcase( i==11 ); /* IGNORE */
      testcase( i==12 ); /* REGEXP */
      testcase( i==13 ); /* EXPLAIN */
      testcase( i==14 ); /* INSTEAD */
      testcase( i==15 ); /* ADD */
      testcase( i==16 ); /* DATABASE */
      testcase( i==17 ); /* AS */
      testcase( i==18 ); /* SELECT */
      testcase( i==19 ); /* TABLE */
      testcase( i==20 ); /* LEFT */
      testcase( i==21 ); /* THEN */
      testcase( i==22 ); /* END */
      testcase( i==23 ); /* DEFERRABLE */
      testcase( i==24 ); /* ELSE */
      testcase( i==25 ); /* EXCEPT */
      testcase( i==26 ); /* TRANSACTION */
      testcase( i==27 ); /* ACTION */
      testcase( i==28 ); /* ON */
      testcase( i==29 ); /* NATURAL */
      testcase( i==30 ); /* ALTER */
      testcase( i==31 ); /* RAISE */
      testcase( i==32 ); /* EXCLUSIVE */
      testcase( i==33 ); /* EXISTS */
      testcase( i==34 ); /* SAVEPOINT */
      testcase( i==35 ); /* INTERSECT */
      testcase( i==36 ); /* TRIGGER */
      testcase( i==37 ); /* REFERENCES */
      testcase( i==38 ); /* CONSTRAINT */
      testcase( i==39 ); /* INTO */
      testcase( i==40 ); /* OFFSET */
      testcase( i==41 ); /* OF */
      testcase( i==42 ); /* SET */
      testcase( i==43 ); /* TEMPORARY */
      testcase( i==44 ); /* TEMP */
      testcase( i==45 ); /* OR */
      testcase( i==46 ); /* UNIQUE */
      testcase( i==47 ); /* QUERY */
      testcase( i==48 ); /* WITHOUT */
      testcase( i==49 ); /* OUTER */
      testcase( i==50 ); /* RELEASE */
      testcase( i==51 ); /* ATTACH */
      testcase( i==52 ); /* HAVING */
      testcase( i==53 ); /* GROUP */
      testcase( i==54 ); /* UPDATE */
      testcase( i==55 ); /* BEGIN */
      testcase( i==56 ); /* INNER */
      testcase( i==57 ); /* RENAME */
      testcase( i==58 ); /* BETWEEN */
      testcase( i==59 ); /* NOTNULL */
      testcase( i==60 ); /* NOT */
      testcase( i==61 ); /* NO */
      testcase( i==62 ); /* NULL */
      testcase( i==63 ); /* LIKE */
      testcase( i==64 ); /* CASCADE */
      testcase( i==65 ); /* ASC */
      testcase( i==66 ); /* DELETE */
      testcase( i==67 ); /* CASE */
      testcase( i==68 ); /* COLLATE */
      testcase( i==69 ); /* CREATE */
      testcase( i==70 ); /* CURRENT_DATE */
      testcase( i==71 ); /* DETACH */
      testcase( i==72 ); /* IMMEDIATE */
      testcase( i==73 ); /* JOIN */
      testcase( i==74 ); /* INSERT */
      testcase( i==75 ); /* MATCH */
      testcase( i==76 ); /* PLAN */
      testcase( i==77 ); /* ANALYZE */
      testcase( i==78 ); /* PRAGMA */
      testcase( i==79 ); /* ABORT */
      testcase( i==80 ); /* VALUES */
      testcase( i==81 ); /* VIRTUAL */
      testcase( i==82 ); /* LIMIT */
      testcase( i==83 ); /* WHEN */
      testcase( i==84 ); /* WHERE */
      testcase( i==85 ); /* REPLACE */
      testcase( i==86 ); /* AFTER */
      testcase( i==87 ); /* RESTRICT */
      testcase( i==88 ); /* AND */
      testcase( i==89 ); /* DEFAULT */
      testcase( i==90 ); /* AUTOINCREMENT */
      testcase( i==91 ); /* TO */
      testcase( i==92 ); /* IN */
      testcase( i==93 ); /* CAST */
      testcase( i==94 ); /* COLUMN */
      testcase( i==95 ); /* COMMIT */
      testcase( i==96 ); /* CONFLICT */
      testcase( i==97 ); /* CROSS */
      testcase( i==98 ); /* CURRENT_TIMESTAMP */
      testcase( i==99 ); /* CURRENT_TIME */
      testcase( i==100 ); /* PRIMARY */
      testcase( i==101 ); /* DEFERRED */
      testcase( i==102 ); /* DISTINCT */
      testcase( i==103 ); /* IS */
      testcase( i==104 ); /* DROP */
      testcase( i==105 ); /* FAIL */
      testcase( i==106 ); /* FROM */
      testcase( i==107 ); /* FULL */
      testcase( i==108 ); /* GLOB */
      testcase( i==109 ); /* BY */
      testcase( i==110 ); /* IF */
      testcase( i==111 ); /* ISNULL */
      testcase( i==112 ); /* ORDER */
      testcase( i==113 ); /* RIGHT */
      testcase( i==114 ); /* ROLLBACK */
      testcase( i==115 ); /* ROW */
      testcase( i==116 ); /* UNION */
      testcase( i==117 ); /* USING */
      testcase( i==118 ); /* VACUUM */
      testcase( i==119 ); /* VIEW */
      testcase( i==120 ); /* INITIALLY */
      testcase( i==121 ); /* ALL */
      return aCode[i];
    }
  }
  return TK_ID;
}
int sqlite3KeywordCode(const unsigned char *z, int n){
  return keywordCode((char*)z, n);
}
#define SQLITE_N_KEYWORD 122
