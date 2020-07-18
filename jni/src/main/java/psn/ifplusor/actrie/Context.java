package psn.ifplusor.actrie;

import java.util.Iterator;

public class Context implements Iterable<Word>, AutoCloseable {

    private final Matcher matcher;
    private long nativeContext = 0;

    private String content = null;
    private boolean returnBytePos = false;

    private boolean uninitialized = false;

    public Context(Matcher matcher) throws MatcherError {
        if (matcher == null) {
            throw new MatcherError("Matcher is null");
        }

        this.matcher = matcher;
        // alloc native context
        this.nativeContext = Context.AllocContext(this.matcher.getNativeMatcher());
    }

    public Context(Matcher matcher, String content) throws MatcherError {
        this(matcher, content, false);
    }

    public Context(Matcher matcher, String content, boolean returnBytePos) throws MatcherError {
        this(matcher);
        this.reset(content, returnBytePos);
    }

    public void reset(String content, Boolean returnBytePos) throws MatcherError {
        if (content != null) {
            this.content = content;
        }
        if (returnBytePos != null) {
            this.returnBytePos = returnBytePos;
        }
        this.uninitialized = !Context.ResetContext(this.nativeContext, this.content, this.returnBytePos);
        if (this.uninitialized) {
            throw new MatcherError("Reset context failed!");
        }
    }

    public Word next() {
        if (this.uninitialized) {
            return null;
        }
        return Context.Next(this.nativeContext);
    }

    @Override
    public Iterator<Word> iterator() {
        return new WordIterator();
    }

    public class WordIterator implements Iterator<Word> {
        private Word nextWord = null;

        private WordIterator() {
        }

        @Override
        public boolean hasNext() {
            this.nextWord = Context.this.next();
            return this.nextWord != null;
        }

        @Override
        public Word next() {
            return this.nextWord;
        }
    }

    @Override
    public void close() throws Exception {
        Context.FreeContext(this.nativeContext);
        this.nativeContext = 0;
    }

    @Override
    protected void finalize() throws Throwable {
        this.close();
    }

    // use in JNI
    public static Word buildWord(String keyword, long so, long eo, String extra) {
        return new Word(keyword, so, eo, extra);
    }

    private static native long AllocContext(long matcher);

    private static native boolean FreeContext(long context);

    private static native boolean ResetContext(long context, String content, boolean returnBytePos);

    private static native Word Next(long context);

}
