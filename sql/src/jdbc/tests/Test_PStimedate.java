import java.sql.*;

public class Test_PStimedate {
	public static void main(String[] args) throws Exception {
		Class.forName("nl.cwi.monetdb.jdbc.MonetDriver");
		//nl.cwi.monetdb.jdbc.MonetConnection.setDebug(true);
		Connection con = DriverManager.getConnection("jdbc:monetdb://localhost/database", "monetdb", "monetdb");
		Statement stmt = con.createStatement();
		PreparedStatement pstmt;
		ResultSet rs = null;
		//DatabaseMetaData dbmd = con.getMetaData();

		con.setAutoCommit(false);
		// >> false: auto commit was just switched off
		System.out.println("false\t" + con.getAutoCommit());

		try {
			stmt.executeUpdate("CREATE TABLE table_Test_PStimedate (t time, ts timestamp, d date)");
		} catch (SQLException e) {
			System.out.println(e);
			System.out.println("Creation of test table failed! :(");
			System.out.println("ABORTING TEST!!!");
			System.exit(-1);
		}

		try {
			pstmt = con.prepareStatement("INSERT INTO table_Test_PStimedate VALUES (?, ?, ?)");
			System.out.print("empty call...");
			try {
				// should fail (no arguments given)
				pstmt.execute();
				System.out.println(" PASSED :(");
				System.out.println("ABORTING TEST!!!");
				System.exit(-1);
			} catch (SQLException e) {
				System.out.println(" failed :)");
			}

			System.out.print("inserting a record...");
			java.util.Date d = new java.util.Date();
			pstmt.setTime(1, new java.sql.Time(d.getTime()));
			pstmt.setTimestamp(2, new java.sql.Timestamp(d.getTime()));
			pstmt.setDate(3, new java.sql.Date(d.getTime()));

			pstmt.executeUpdate();
			System.out.println(" done");
			System.out.print("closing PreparedStatement...");
			pstmt.close();
			System.out.println(" done");

			System.out.print("selecting record...");
			pstmt = con.prepareStatement("SELECT * FROM table_Test_PStimedate");
			rs = pstmt.executeQuery();
			System.out.println(" done");

			while (rs.next()) {
				for (int j = 1; j <= 3; j++) {
					System.out.print("retrieving...");
					java.util.Date x = (java.util.Date)(rs.getObject(j));
					boolean matches = false;
					if (x instanceof Time) {
						matches = (new Time(d.getTime())).toString().equals(x.toString());
					} else if (x instanceof Date) {
						matches = (new Date(d.getTime())).toString().equals(x.toString());
					} else if (x instanceof Timestamp) {
						matches = (new Timestamp(d.getTime())).toString().equals(x.toString());
					}
					if (matches) {
						System.out.println(" passed :)");
					} else {
						System.out.println(" FAILED :(");
					}
				}
			}

			con.rollback();
		} catch (SQLException e) {
			System.out.println("FAILED :( "+ e.getMessage());
			System.out.println("ABORTING TEST!!!");
		}

		con.close();
	}
}
