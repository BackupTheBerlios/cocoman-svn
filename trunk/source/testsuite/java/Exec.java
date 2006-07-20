import java.lang.Runtime;

public class Exec{
    public static void main(String[] args) throws java.io.IOException{
        Runtime run = Runtime.getRuntime();
        run.exec("ping");
    }
}
