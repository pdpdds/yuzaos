void splitpath(const char* path, char* drv, char* dir, char* name, char* ext)	
{
         const char* end; /* end of processed string */
     const char* p;   /* search pointer */
     const char* s;   /* copy pointer */
 
     /* extract drive name */
     if (path[0] && path[1]==':') {
         if (drv) {
             *drv++ = *path++;
             *drv++ = *path++;
             *drv = '\0';
         }
     } else if (drv)
         *drv = '\0';
 
     /* Don't parse colons as stream separators when splitting paths */
     end = path + strlen(path);
 
     /* search for begin of file extension */
     for(p=end; p>path && *--p!='\\' && *p!='/'; )
         if (*p == '.') {
             end = p;
             break;
         }
 
     if (ext)
         for(s=end; (*ext=*s++); )
             ext++;
 
     /* search for end of directory name */
     for(p=end; p>path; )
         if (*--p=='\\' || *p=='/') {
             p++;
             break;
         }
 
     if (name) {
         for(s=p; s<end; )
             *name++ = *s++;
 
         *name = '\0';
     }
 
     if (dir) {
         for(s=path; s<p; )
             *dir++ = *s++;
 
         *dir = '\0';
     }
 }