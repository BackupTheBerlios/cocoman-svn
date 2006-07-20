import java.io.*;

public class FS {
    public static void main(String[] args) throws java.io.IOException{
        File file = new File("somefile");
        file.createNewFile();
        file.delete();
    }
}
