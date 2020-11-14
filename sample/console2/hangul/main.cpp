#include <stdio.h>
#include <hangul.h>

int main(void)
{
    int i;
    char buf[16];
    char input[] = { 'g', 'k', 's', 'r', 'm', 'f', '\0' }; /* ÇÑ±Û */
    HangulInputContext* hic;

    hic = hangul_ic_new("2");
    for (i = 0; input[i]; i++)
    {
        hangul_ic_process(hic, input[i]);
        printf("input: %c, committed: 0x%04X, preedit: 0x%04X\n",
            input[i], *hangul_ic_get_commit_string(hic), *hangul_ic_get_preedit_string(hic));
    }

    if (!hangul_ic_is_empty(hic))
        printf("flushed: 0x%04X\n", *hangul_ic_flush(hic));

    hangul_ic_delete(hic);
    return 0;
}