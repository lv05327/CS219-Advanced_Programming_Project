import java.io.*;
import java.util.*;

enum DataType {
    INT, FLOAT, DOUBLE, SHORT, SIGNEDCHAR, UNKNOWN
}

public class dot {

    // 获取数据类型
    public static DataType getDataType(String datatype) {
        switch (datatype) {
            case "int":
                return DataType.INT;
            case "float":
                return DataType.FLOAT;
            case "double":
                return DataType.DOUBLE;
            case "short":
                return DataType.SHORT;
            case "signed char":
                return DataType.SIGNEDCHAR;
            default:
                return DataType.UNKNOWN;
        }
    }

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);
        System.out.print("Enter filename: ");
        String filename = scanner.nextLine();

        try (BufferedReader reader = new BufferedReader(new FileReader(filename))) {
            // 读取数据类型
            String datatype = reader.readLine().trim();

            // 读取向量大小
            int size = Integer.parseInt(reader.readLine().trim());

            // 读取两行向量
            String line1 = reader.readLine().trim();
            String line2 = reader.readLine().trim();

            // 根据数据类型处理
            switch (getDataType(datatype)) {
                case INT:
                case SHORT: {
                    long[] vector1 = parseLongArray(line1, size);
                    long[] vector2 = parseLongArray(line2, size);

                    // 计算点积
                    long dotProduct = 0;
                    for (int i = 0; i < size; i++) {
                        dotProduct += vector1[i] * vector2[i];
                    }
                    System.out.println("Dot product: " + dotProduct);
                    break;
                }
                case FLOAT:
                case DOUBLE: {
                    double[] vector1 = parseDoubleArray(line1, size);
                    double[] vector2 = parseDoubleArray(line2, size);

                    // 计算点积
                    double dotProduct = 0;
                    for (int i = 0; i < size; i++) {
                        dotProduct += vector1[i] * vector2[i];
                    }
                    System.out.printf("Dot product: %.2f\n", dotProduct);
                    break;
                }
                case SIGNEDCHAR: {
                    long[] vector1 = parseCharArray(line1, size);
                    long[] vector2 = parseCharArray(line2, size);

                    // 计算点积
                    long dotProduct = 0;
                    for (int i = 0; i < size; i++) {
                        dotProduct += vector1[i] * vector2[i];
                    }
                    System.out.println("Dot product: " + dotProduct);
                    break;
                }
                default:
                    System.out.println("Unsupported data type: " + datatype);
            }
        } catch (IOException e) {
            System.out.println("Error reading file: " + e.getMessage());
        } catch (NumberFormatException e) {
            System.out.println("Error parsing number: " + e.getMessage());
        }
    }

    // 解析 long 数组
    private static long[] parseLongArray(String line, int size) {
        String[] tokens = line.split(",");
        long[] array = new long[size];
        for (int i = 0; i < size; i++) {
            array[i] = Long.parseLong(tokens[i].trim());
        }
        return array;
    }

    // 解析 double 数组
    private static double[] parseDoubleArray(String line, int size) {
        String[] tokens = line.split(",");
        double[] array = new double[size];
        for (int i = 0; i < size; i++) {
            array[i] = Double.parseDouble(tokens[i].trim());
        }
        return array;
    }

    // 解析 signed char 数组
    private static long[] parseCharArray(String line, int size) {
        String[] tokens = line.split(",");
        long[] array = new long[size];
        for (int i = 0; i < size; i++) {
            // 提取单引号中的字符并转换为 ASCII 值
            array[i] = tokens[i].trim().charAt(1);
        }
        return array;
    }
}