import firebase_admin
from firebase_admin import credentials, db
import matplotlib.pyplot as plt
import mplcursors
from dotenv import load_dotenv
import os

load_dotenv()

class PacePlotter:
    def __init__(self, date):
        self.cred_path = os.environ['FIREBASE_CRED_PATH']
        self.db_url = os.environ['FIREBASE_DB_URL']
        self.date_path = f'/run/{date}/'
        self.lats = []
        self.lngs = []
        self.paces = []
        self.distances = []
        
        self._initialize_firebase()
        self._retrieve_data()
    
    def _initialize_firebase(self):
        # Initialize Firebase Admin SDK
        cred = credentials.Certificate(self.cred_path)
        firebase_admin.initialize_app(cred, {
            'databaseURL': self.db_url
        })
    
    def _retrieve_data(self):
        # Retrieve all data in the specified path
        ref = db.reference(self.date_path)
        data = ref.get()
        
        # Collect latitude, longitude, pace, and total distance values
        for timestamp, values in data.items():
            self.lats.append(values['lat'])
            self.lngs.append(values['lng'])
            self.paces.append(values['pace'])
            self.distances.append(values['totalDistance'])
    
    def plot_data(self):
        # Plot the lat/lng points with colors based on pace
        plt.figure(figsize=(10, 6))
        sc = plt.scatter(self.lngs, self.lats, c=self.paces, cmap='viridis', s=100, alpha=0.7)
        plt.colorbar(sc, label='Pace (min/km)')
        plt.xlabel('Longitude')
        plt.ylabel('Latitude')
        plt.title(f'Running Path for {self.date_path.split("/")[2]}')
        
        # Add hover functionality to display lat, lng, pace, and total distance
        cursor = mplcursors.cursor(sc, hover=True)
        cursor.connect("add", self._on_add)
        
        # Show plot
        plt.show()

    def _on_add(self, sel):
        index = sel.index
        sel.annotation.set(text=f"Lat: {self.lats[index]:.2f}\nLng: {self.lngs[index]:.2f}\n"
                                f"Pace: {self.paces[index]:.2f} min/km\nDistance: {self.distances[index]:.3f} km",
                           bbox=dict(boxstyle="round,pad=0.3", fc="yellow", alpha=1))


if __name__ == "__main__":
    plotter = PacePlotter(
        date='26-08-2024'
    )
    
    plotter.plot_data()
