package dk.thoerup.osrmbinding;

public class OSRMBinding {

	static {
		System.loadLibrary("osrmjavabindings");
	}

	public long ptr;


	/**
	 * Create and initialize libosrm for using shared mem access.
	 * osrm-datastore(1) must be running.
	 */
	public OSRMBinding() {
		this.init(true, "");
	}

	/**
	 * Create and initialize libosrm for using disk based access to data files.
	 */
	public OSRMBinding(String path) {
		this.init(false, path);
	}

	@Override
	public void finalize() {
		this.destroy();
	}


	private native void init(boolean sharedMem, String path); 
	private native void destroy();

	public native ViarouteResult viaRoute(Geopoint points[]);
	public native float[][] table(Geopoint points[]);



	//test driver
	public static void main(String[] args) {
		//OSRMBinding osrm = new OSRMBinding();//Shared mem
		OSRMBinding osrm = new OSRMBinding("/home/openstreetmap/denmark-latest.osrm");


		Geopoint points[] = new Geopoint[ 2 ];
		points[0] = new Geopoint(55.8585089,9.8410706);
		points[1] = new Geopoint( 55.7131689,9.5417373);

		ViarouteResult res = osrm.viaRoute( points);
		System.out.println("viaRoute returned: " + res);

		for(int i=0; i<4; i++) {
			testTable(osrm);
		}
	}

	public static void testTable(OSRMBinding osrm) {
		Geopoint points[] = new Geopoint[4];
		points[0] = new Geopoint(56.1857366, 9.5725776);
		points[1] = new Geopoint(55.8729351, 9.8271557);
		points[2] = new Geopoint(55.7259257, 9.5701175);
		points[3] = new Geopoint(56.0430661, 9.925867);
		
		float[][] table = osrm.table(points);
		System.out.println("table returned: ");
		for (int i=0; i<table.length; i++) {
			for (int j=0; j<table[i].length; j++) {
				System.out.print(table[i][j]);
				System.out.print(" ");
			}
			System.out.println();
		}
	}
} 

