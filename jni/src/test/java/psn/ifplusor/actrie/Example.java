package psn.ifplusor.actrie;

public class Example {

    public static void main(String[] args) {
        String pattern = "中国\t1\n北京\t2";
        String content = "中国，北京";

        try (Matcher matcher = Matcher.createByString(pattern)) {
            try (Context context = matcher.match(content)) {
                for (Word word : context) {
                    System.out.println(word);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

}
