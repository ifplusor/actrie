package psn.ifplusor.actrie;

public class MatcherError extends Exception {

    private static final long serialVersionUID = 2926842074720533412L;

    public MatcherError() {
        super();
    }

    public MatcherError(String message) {
        super(message);
    }

    public MatcherError(String message, Throwable cause) {
        super(message, cause);
    }

    public MatcherError(Throwable cause) {
        super(cause);
    }
}
