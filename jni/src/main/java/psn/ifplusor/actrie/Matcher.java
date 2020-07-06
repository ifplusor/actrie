package psn.ifplusor.actrie;

public class Matcher implements AutoCloseable {

    static {
        NarSystem.loadLibrary();
    }

    private long nativeMatcher = 0;

    public long getNativeMatcher() {
        return this.nativeMatcher;
    }

    public static Matcher createByFile(String filepath) throws MatcherError {
        return Matcher.createByFile(filepath, false, false, true, true);
    }

    public static Matcher createByFile(String filepath, boolean allAsPlain, boolean ignoreBadPattern,
            boolean badAsPlain, boolean deduplicateExtra) throws MatcherError {
        Matcher matcher = new Matcher();
        if (matcher.loadFromFile(filepath, allAsPlain, ignoreBadPattern, badAsPlain, deduplicateExtra)) {
            return matcher;
        }
        return null;
    }

    public boolean loadFromFile(String filepath) throws MatcherError {
        return loadFromFile(filepath, false, false, true, true);
    }

    public boolean loadFromFile(String filepath, boolean allAsPlain, boolean ignoreBadPattern, boolean badAsPlain,
            boolean deduplicateExtra) throws MatcherError {
        if (this.nativeMatcher != 0) {
            throw new MatcherError("Matcher is already initialized.");
        }
        if (filepath == null) {
            return false;
        }
        this.nativeMatcher = Matcher.ConstructByFile(filepath, allAsPlain, ignoreBadPattern, badAsPlain,
                deduplicateExtra);
        return this.nativeMatcher != 0;
    }

    public static Matcher createByString(String keywords) throws MatcherError {
        return Matcher.createByString(keywords, false, false, true, true);
    }

    public static Matcher createByString(String keywords, boolean allAsPlain, boolean ignoreBadPattern,
            boolean badAsPlain, boolean deduplicateExtra) throws MatcherError {
        Matcher matcher = new Matcher();
        if (matcher.loadFromString(keywords, allAsPlain, ignoreBadPattern, badAsPlain, deduplicateExtra)) {
            return matcher;
        }
        return null;
    }

    public boolean loadFromString(String keywords) throws MatcherError {
        return loadFromString(keywords, false, false, true, true);
    }

    public boolean loadFromString(String keywords, boolean allAsPlain, boolean ignoreBadPattern, boolean badAsPlain,
            boolean deduplicateExtra) throws MatcherError {
        if (this.nativeMatcher != 0) {
            throw new MatcherError("Matcher is already initialized.");
        }
        if (keywords == null) {
            return false;
        }
        this.nativeMatcher = Matcher.ConstructByString(keywords, allAsPlain, ignoreBadPattern, badAsPlain,
                deduplicateExtra);
        return this.nativeMatcher != 0;
    }

    public Context match(String content) throws MatcherError {
        return match(content, false);
    }

    public Context match(String content, boolean returnBytePos) throws MatcherError {
        return new Context(this, content, returnBytePos);
    }

    @Override
    public void close() throws Exception {
        Matcher.Destruct(this.nativeMatcher);
        this.nativeMatcher = 0;
    }

    private static native long ConstructByFile(String filepath, boolean allAsPlain, boolean ignoreBadPattern,
            boolean badAsPlain, boolean deduplicateExtra);

    private static native long ConstructByString(String keywords, boolean allAsPlain, boolean ignoreBadPattern,
            boolean badAsPlain, boolean deduplicateExtra);

    private static native boolean Destruct(long matcher);

}
