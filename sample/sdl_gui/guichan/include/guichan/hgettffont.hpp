#ifndef GCN_HGETTFFONT_HPP
#define GCN_HGETTFFONT_HPP

#include <string>
//#include <hgeTTF/FontManager.h> //hgeTTF 라이브러리를 인클루드
//#include <hgeTTF/Font.h>

#include "guichan/font.hpp"
#include "guichan/platform.hpp"

namespace gcn
{
    class Graphics;

	//HGE TTF 라이브러리를 이용한 폰트 클래스
	//100111 유니코드 변경
    class GCN_EXTENSION_DECLSPEC HGETTFFont : public Font
    {
    public:
        
		//HGE TTF 폰트 객체 포인터를 인자로 받음
        HGETTFFont(hgeTTF::Font* font);
        ~HGETTFFont();
        virtual hgeTTF::Font *getFont() const;
        int getWidth(wchar_t character) const;


        //Font로부터 상속받은 함수들.
        int getWidth(const std::wstring& text) const;
        int getHeight() const;
        int getStringIndexAt(const std::wstring& text, int x) const;
        void drawString(Graphics *graphics, const std::wstring &text, int x, int y);

    protected:
        hgeTTF::Font *mHGEFont;
    };
}

#endif // end GCN_HGETTFFONT_HPP