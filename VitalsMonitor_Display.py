import tkinter as tk
from tkinter import filedialog
import pandas as pd
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import time
import numpy as np
import neurokit2 as nk

class VitalSignsMonitor:
    def __init__(self, root, data):
        self.root = root
        self.root.title("Vital Signs Monitor")
        self.root.configure(bg='black')  # Set the background color to black
        self.start_time = time.time()  # Record the start time when data is loaded
        self.data = data
        self.current_index = 0
        self.oxygen_saturation_values = []
        self.times = []
        self.heart_rate_times = []

        # Layout configuration: 2 columns, 3 rows
        self.root.grid_columnconfigure(0, weight=1)
        self.root.grid_columnconfigure(1, weight=1)
        self.root.grid_rowconfigure(0, weight=1)
        self.root.grid_rowconfigure(1, weight=1)
        self.root.grid_rowconfigure(2, weight=1)

        # Placeholder for matplotlib figures
        self.fig, self.axs = plt.subplots(3, 1, figsize=(8, 9))
        self.fig.patch.set_facecolor('black')  # Set figure background to black
        self.canvas = FigureCanvasTkAgg(self.fig, master=root)
        self.canvas.get_tk_widget().grid(row=0, column=0, rowspan=3, sticky="nsew")

        # Labels for displaying current vital sign values with increased font size
        self.hr_label = tk.Label(root, text="", font=("Helvetica", 36), fg="lime", bg='black')
        self.hr_label.grid(row=0, column=1, sticky="nsew", padx=20, pady=20)

        self.bp_label = tk.Label(root, text="", font=("Helvetica", 36), fg="red", bg='black')
        self.bp_label.grid(row=1, column=1, sticky="nsew", padx=20, pady=20)

        self.o2_label = tk.Label(root, text="", font=("Helvetica", 36), fg="blue", bg='black')
        self.o2_label.grid(row=2, column=1, sticky="nsew", padx=20, pady=20)

        self.update_plot_loop()

    def generate_neurokit_ecg(self, heart_rate, duration=5):
        """Generate ECG signal using NeuroKit."""
        ecg_signal = nk.ecg_simulate(duration=duration, heart_rate=heart_rate, noise=0.01)
        time_range = np.linspace(0, duration, len(ecg_signal))
        return time_range, ecg_signal

    def update_plot_loop(self):
        if self.data is not None:
            elapsed_time = time.time() - self.start_time

            # Check if it's time to update to the next set of values
            if self.current_index < len(self.data) and elapsed_time >= self.data.iloc[self.current_index, 0]:
                heart_rate = self.data.iloc[self.current_index, 1]  # Second column
                systolic_bp = self.data.iloc[self.current_index, 2]  # Third column
                diastolic_bp = self.data.iloc[self.current_index, 3]  # Fourth column
                oxygen_saturation = self.data.iloc[self.current_index, 4]  # Fifth column

                # Update oxygen saturation values and times
                self.oxygen_saturation_values.append(oxygen_saturation)
                self.times.append(self.data.iloc[self.current_index, 0])

                # Clear previous plots
                for ax in self.axs:
                    ax.clear()

                # Plot NeuroKit ECG simulation in the first and second rows (neon green)
                if heart_rate > 0:
                    time_range, ecg_signal = self.generate_neurokit_ecg(heart_rate)
                    self.axs[0].plot(time_range + self.times[-1], ecg_signal, 'lime', lw=1.5, label='EKG Pulse')  # Neon green
                    self.axs[0].set_facecolor('black')
                    self.axs[1].plot(time_range + self.times[-1], ecg_signal, 'lime', label='EKG Pulse')  # Neon green
                    self.axs[1].set_facecolor('black')

                # Plot Oxygen Saturation as a line graph (bright blue)
                self.axs[2].plot(self.times, self.oxygen_saturation_values, 'b-', marker='o')
                self.axs[2].set_facecolor('black')

                # Apply black background to the plots and set white elements
                for ax in self.axs:
                    ax.set_facecolor('black')
                    ax.spines['bottom'].set_color('white')
                    ax.spines['top'].set_color('white')
                    ax.spines['right'].set_color('white')
                    ax.spines['left'].set_color('white')
                    ax.yaxis.label.set_color('white')
                    ax.xaxis.label.set_color('white')
                    ax.tick_params(axis='y', colors='white')
                    ax.tick_params(axis='x', colors='white')

                # Update labels with current values without adding labels
                self.hr_label.config(text=f"{heart_rate:.1f} BPM")
                self.bp_label.config(text=f"{systolic_bp:.1f} / {diastolic_bp:.1f}")
                self.o2_label.config(text=f"{oxygen_saturation:.1f}%")

                # Increment the index to move to the next set of values
                self.current_index += 1

                # Refresh the plot on the canvas
                self.canvas.draw()

        # Repeat the update every 100ms
        self.root.after(100, self.update_plot_loop)

def load_data_popup():
    # Create a pop-up window for file upload
    popup = tk.Tk()
    popup.title("Upload Data")
    popup.configure(bg='black')

    def load_data_and_close():
        file_path = filedialog.askopenfilename()
        if file_path:
            popup.destroy()
            data = pd.read_excel(file_path, header=0).astype(float)  # Load data and convert to floats
            root = tk.Tk()
            app = VitalSignsMonitor(root, data)
            root.mainloop()

    label = tk.Label(popup, text="Please upload your data file", font=("Helvetica", 16), fg="white", bg='black')
    label.pack(pady=20)

    upload_btn = tk.Button(popup, text="Upload Data", command=load_data_and_close, bg='grey', fg='white', font=("Helvetica", 12))
    upload_btn.pack(pady=20)

    popup.mainloop()

if __name__ == "__main__":
    load_data_popup()
