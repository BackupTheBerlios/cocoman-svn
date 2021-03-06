import java.lang.reflect.*;
import java.io.File;
import java.util.StringTokenizer;
import java.io.*;
import java.security.Policy;
import java.lang.SecurityManager;

class JavaRun {
    public static void main(String[] args) {
        int i;
        File dotslash = new File("./");
        String[] contents = dotslash.list();
        String[] tokens;
        Class defined;
        String classname="";
        Class[] argTypes = new Class[1];
        argTypes[0] = String[].class;
        Object[] parameters = new Object[1];
        parameters[0] = new String[0];
        Object[] pargs = parameters;
        for (i = 0; i < contents.length; i++) {
            try {
                File curfile = new File(contents[i]);
                tokens = contents[i].split("\\.");
                classname = "";
                for (int j = 0; j < tokens.length-1; j++) {
                    classname += tokens[j];
                }
                if (classname.equals("JavaRun")) continue;
                if (curfile.isDirectory()) continue;
//                Method mainMethod = Class.forName(classname).getDeclaredMethod("main", argTypes);
//                  Method mainMethod = loadClass(contents[i]).getDeclaredMethod("main", argTypes);
                  defined = loadClass(contents[i]);
                  Method mainMethod = defined.getDeclaredMethod("main", argTypes);
                  mainMethod.setAccessible(true);
                try {
                    System.setSecurityManager(new SecurityManager());
                    mainMethod.invoke(null, pargs);
                    System.exit(0);
                } catch (IllegalAccessException e) {
//                    System.out.println(e);
                } catch (InvocationTargetException e) {
//                    System.out.println(e);
                } catch (ClassFormatError e) {
//                    System.out.println(e); 
                } finally { 
                }
            } catch (ClassNotFoundException e) {
//                System.out.println(e);
            } catch (NoSuchMethodException e) {
//                System.out.println(e);
            }
        }
    }
   

    // Taken from Raymond DeCampo on http://www.thescripts.com/forum/thread15909.html 
    static Class loadClass(final String filename) throws
        ClassNotFoundException
        {
            ClassLoader loader = new ClassLoader()
            {
                protected Class findClass(String name) throws
                    ClassNotFoundException
                    {
                        // Always load the file
                        InputStream in = null;
                        try
                        {
                            in = new BufferedInputStream(new
                                    FileInputStream(filename));
                            ByteArrayOutputStream byteStr = new
                                ByteArrayOutputStream();
                            int byt = -1;
                            while ((byt = in.read()) != -1)
                            {
                                byteStr.write(byt);
                            }
                            byte byteArr[] = byteStr.toByteArray();
                            return defineClass(null, byteArr, 0, byteArr.length);
                        }
                        catch (final RuntimeException rex)
                        {
                            throw rex;
                        }
                        catch (final Exception ex)
                        {
                            ex.printStackTrace();
                            throw new ClassNotFoundException(name);
                        }
                        catch (ClassFormatError ex) {
                            throw new ClassNotFoundException(name);
                        }
                        finally
                        {
                            if (in != null)
                            {
                                try
                                {
                                    in.close();
                                }
                                catch (final IOException ioe)
                                {
                                    ioe.printStackTrace();
                                }
                            }
                        }
                    }
            };
            return loader.loadClass("garbage");
        }
}

