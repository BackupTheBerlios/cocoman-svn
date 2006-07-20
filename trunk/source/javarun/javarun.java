import java.lang.reflect.*;
import java.io.File;
import java.util.StringTokenizer;
class JavaRun {
    public static void main(String[] args) {
        int i;
        File dotslash = new File("./");
        String[] contents = dotslash.list();
        String[] tokens;
        String classname="";
        Class[] argTypes = new Class[1];
        argTypes[0] = String[].class;
        Object[] parameters = new Object[1];
        parameters[0] = new String[0];
        Object[] pargs = parameters;
        for (i = 0; i < contents.length; i++) {
            try {
                tokens = contents[i].split("\\.");
                classname = "";
                for (int j = 0; j < tokens.length-1; j++) {
                    classname += tokens[j];
                }
                if (classname.equals("Runner")) continue;
                Method mainMethod = Class.forName(classname).getDeclaredMethod("main", argTypes);
                try {
                    mainMethod.invoke(null, pargs);
                } catch (IllegalAccessException e) {
//                    System.out.println(e);
                } catch (InvocationTargetException e) {
//                    System.out.println(e);
                } finally { 
                    break;
                }
            } catch (ClassNotFoundException e) {
//                System.out.println(e);
            } catch (NoSuchMethodException e) {
//                System.out.println(e);
            }
        }
    }
}
            
