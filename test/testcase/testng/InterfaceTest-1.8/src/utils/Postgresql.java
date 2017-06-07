package utils;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;

public class Postgresql {

	public static void  select(String sql){
		Connection connection = getConn(); 
		Statement statement = null;
		try {
			statement = connection.createStatement();
			ResultSet resultSet = statement.executeQuery(sql);
			PreparedStatement pstmt;  
	        try {  
	            pstmt = (PreparedStatement)connection.prepareStatement(sql);  
	            ResultSet rs = pstmt.executeQuery();  
	            int col = rs.getMetaData().getColumnCount();  
	            System.out.println("==================================================================================================");  
	            while (rs.next()) {  
	                for (int i = 1; i <= col; i++) {  
	                    System.out.print(rs.getString(i) + "\t");  
	                    if ((i == 2) && (rs.getString(i).length() < 8)) {  
	                        System.out.print("\t");  
	                    }  
	                 }  
	                System.out.println("");  
	            }  
	                System.out.println("==================================================================================================");  
	        } catch (SQLException e) {  
	            e.printStackTrace();  
	        }  
		} catch (Exception e) {
			throw new RuntimeException(e);
		} finally {
			try {
				statement.close();
			} catch (SQLException e) {
				e.printStackTrace();
				throw new RuntimeException(e);
			} finally {
				try {
					connection.close();
				} catch (SQLException e) {
					e.printStackTrace();
					throw new RuntimeException(e);
				}
			}
		}
	}
	
	public static void  select(String ip,String dbname,String username,String pwd,String sql){
		Connection connection = getConn(ip,dbname,username, pwd); 
		Statement statement = null;
		try {
			statement = connection.createStatement();
			ResultSet resultSet = statement.executeQuery(sql);
			PreparedStatement pstmt;  
	        try {  
	            pstmt = (PreparedStatement)connection.prepareStatement(sql);  
	            ResultSet rs = pstmt.executeQuery();  
	            int col = rs.getMetaData().getColumnCount();  
	            System.out.println("==================================================================================================");  
	            while (rs.next()) {  
	                for (int i = 1; i <= col; i++) {  
	                    System.out.print(rs.getString(i) + "\t");  
	                    if ((i == 2) && (rs.getString(i).length() < 8)) {  
	                        System.out.print("\t");  
	                    }  
	                 }  
	                System.out.println("");  
	            }  
	                System.out.println("==================================================================================================");  
	        } catch (SQLException e) {  
	            e.printStackTrace();  
	        }  
		} catch (Exception e) {
			throw new RuntimeException(e);
		} finally {
			try {
				statement.close();
			} catch (SQLException e) {
				e.printStackTrace();
				throw new RuntimeException(e);
			} finally {
				try {
					connection.close();
				} catch (SQLException e) {
					e.printStackTrace();
					throw new RuntimeException(e);
				}
			}
		}
	}
	
	public static void  select(String dbname,String username,String pwd,String sql){
		String ip = "127.0.0.1:5432";
		Connection connection = getConn(ip,dbname,username, pwd); 
		Statement statement = null;
		try {
			statement = connection.createStatement();
			ResultSet resultSet = statement.executeQuery(sql);
			PreparedStatement pstmt;  
	        try {  
	            pstmt = (PreparedStatement)connection.prepareStatement(sql);  
	            ResultSet rs = pstmt.executeQuery();  
	            int col = rs.getMetaData().getColumnCount();  
	            System.out.println("==================================================================================================");  
	            while (rs.next()) {  
	                for (int i = 1; i <= col; i++) {  
	                    System.out.print(rs.getString(i) + "\t");  
	                    if ((i == 2) && (rs.getString(i).length() < 8)) {  
	                        System.out.print("\t");  
	                    }  
	                 }  
	                System.out.println("");  
	            }  
	                System.out.println("==================================================================================================");  
	        } catch (SQLException e) {  
	            e.printStackTrace();  
	        }  
		} catch (Exception e) {
			throw new RuntimeException(e);
		} finally {
			try {
				statement.close();
			} catch (SQLException e) {
				e.printStackTrace();
				throw new RuntimeException(e);
			} finally {
				try {
					connection.close();
				} catch (SQLException e) {
					e.printStackTrace();
					throw new RuntimeException(e);
				}
			}
		}
	}
	
	public int delete(String ip,String dbname,String username,String pwd,String sql) {  
        Connection conn = getConn(ip,dbname,username, pwd);  
        int i = 0;  
        PreparedStatement pstmt;  
        try {  
            pstmt = (PreparedStatement) conn.prepareStatement(sql);  
            i = pstmt.executeUpdate();  
            System.out.println("resutl: " + i);  
            pstmt.close();  
            conn.close();  
        } catch (SQLException e) {  
            e.printStackTrace();  
        }  
        return i;  
    } 
	
	public int delete(String sql) {  
        Connection conn = getConn();  
        int i = 0;  
        PreparedStatement pstmt;  
        try {  
            pstmt = (PreparedStatement) conn.prepareStatement(sql);  
            i = pstmt.executeUpdate();  
            System.out.println("resutl: " + i);  
            pstmt.close();  
            conn.close();  
        } catch (SQLException e) {  
            e.printStackTrace();  
        }  
        return i;  
    } 
	
	public static Connection getConn(String ip,String dbname,String uname,String pwd) {  
        String driver = "org.postgresql.Driver";  
        String url = "jdbc:postgresql://"+ip+"/"+ dbname;  
        String username = uname;  
        String password = pwd;  
        Connection conn = null;  
        try {  
            Class.forName(driver); // classLoader,加载对应驱动  
            conn = (Connection) DriverManager.getConnection(url, username, password);  
        } catch (ClassNotFoundException e) {  
            e.printStackTrace();  
        } catch (SQLException e) {  
            e.printStackTrace();  
        }  
        return conn;  
    }
	
	public static Connection getConn() {  
        String driver = "org.postgresql.Driver";  
        String url = "jdbc:postgresql://172.16.11.128:5432/_2peer_with_slave_peer1";
		String user = "postgres";
		String password = "root"; 
        Connection conn = null;  
        try {  
            Class.forName(driver); // classLoader,加载对应驱动  
            conn = (Connection) DriverManager.getConnection(url, user, password);  
        } catch (ClassNotFoundException e) {  
            e.printStackTrace();  
        } catch (SQLException e) {  
            e.printStackTrace();  
        }  
        return conn;  
    }
	
	public static void  selectDemo(){
		Connection connection = null;
		Statement statement = null;
		try {
			String url = "jdbc:postgresql://172.16.11.134:5432/_2peer_with_slave_peer1";
			String user = "postgres";
			String password = "postgres"; 
			Class.forName("org.postgresql.Driver");
			connection = DriverManager.getConnection(url, user, password);
			String sql = "SELECT * FROM peers_c;";
			statement = connection.createStatement();
			ResultSet resultSet = statement.executeQuery(sql);
			PreparedStatement pstmt;  
	        try {  
	            pstmt = (PreparedStatement)connection.prepareStatement(sql);  
	            ResultSet rs = pstmt.executeQuery();  
	            int col = rs.getMetaData().getColumnCount();  
	            System.out.println("==================================================================================================");  
	            while (rs.next()) {  
	                for (int i = 1; i <= col; i++) {  
	                    System.out.print(rs.getString(i) + "\t");  
	                    if ((i == 2) && (rs.getString(i).length() < 8)) {  
	                        System.out.print("\t");  
	                    }  
	                 }  
	                System.out.println("");  
	            }  
	                System.out.println("==================================================================================================");  
	        } catch (SQLException e) {  
	            e.printStackTrace();  
	        }  
		} catch (Exception e) {
			throw new RuntimeException(e);
		} finally {
			try {
				statement.close();
			} catch (SQLException e) {
				e.printStackTrace();
				throw new RuntimeException(e);
			} finally {
				try {
					connection.close();
				} catch (SQLException e) {
					e.printStackTrace();
					throw new RuntimeException(e);
				}
			}
		}
	}
}
