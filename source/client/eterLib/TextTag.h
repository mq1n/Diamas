enum
{
	TEXT_TAG_PLAIN,
	TEXT_TAG_TAG, // ||
	TEXT_TAG_COLOR, // |cffffffff
	TEXT_TAG_HYPERLINK_START, // |H
	TEXT_TAG_HYPERLINK_END, // |h ex) |Hitem:1234:1:1:1|h
	TEXT_TAG_RESTORE_COLOR,
};

extern int32_t GetTextTag(const wchar_t * src, int32_t maxLen, int32_t & tagLen, std::wstring & extraInfo);
extern std::wstring GetTextTagOutputString(const wchar_t * src, int32_t src_len);
extern int32_t GetTextTagOutputLen(const wchar_t * src, int32_t len);
extern int32_t FindColorTagEndPosition(const wchar_t * src, int32_t src_len);
extern int32_t FindColorTagStartPosition(const wchar_t * src, int32_t src_len);
extern int32_t GetTextTagInternalPosFromRenderPos(const wchar_t * src, int32_t src_len, int32_t offset);
