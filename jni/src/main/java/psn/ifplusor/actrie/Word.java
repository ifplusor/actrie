package psn.ifplusor.actrie;

public class Word {

    private String keyword;
    private long startOffset;
    private long endOffset;
    private String extra;

    public Word(String keyword, long so, long eo, String extra) {
        this.keyword = keyword;
        this.startOffset = so;
        this.endOffset = eo;
        this.extra = extra;
    }

    public String getKeyword() {
        return keyword;
    }

    public void setKeyword(String keyword) {
        this.keyword = keyword;
    }

    public long getStartOffset() {
        return startOffset;
    }

    public void setStartOffset(long startOffset) {
        this.startOffset = startOffset;
    }

    public long getEndOffset() {
        return endOffset;
    }

    public void setEndOffset(long endOffset) {
        this.endOffset = endOffset;
    }

    public String getExtra() {
        return extra;
    }

    public void setExtra(String extra) {
        this.extra = extra;
    }

    public String toString() {
        return String.format("[%d:%d] %s@%s", this.startOffset, this.endOffset, this.keyword, this.extra);
    }

}
