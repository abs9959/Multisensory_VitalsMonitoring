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
        self.root.configure(bg='black')
        self.start_time = time.time()
        self.data = data
        self.current_index = 0
        self.oxygen_saturation_values = []
        self.times = []

        # Layout configuration: 2 columns, 3 rows
        self.root.grid_columnconfigure(0, weight=1)
        self.root.grid_columnconfigure(1, weight=1)
        self.root.grid_rowconfigure(0, weight=1)
        self.root.grid_rowconfigure(1, weight=1)
        self.root.grid_rowconfigure(2, weight=1)

        # Placeholder for matplotlib figures
        self.fig, self.axs = plt.subplots(3, 1, figsize=(8, 9))
        self.fig.patch.set_facecolor('black')
        self.canvas = FigureCanvasTkAgg(self.fig, master=root)
        self.canvas.get_tk_widget().grid(row=0, column=0, rowspan=3, sticky="nsew")

        # Labels for displaying current vital sign values with increased font size
        self.hr_label = tk.Label(root, text="", font=("Helvetica", 36), fg="lime", bg='black')
        self.hr_label.grid(row=0, column=1, sticky="nsew", padx=20, pady=20)

        self.bp_label = tk.Label(root, text="", font=("Helvetica", 36), fg="red", bg='black')
        self.bp_label.grid(row=1, column=1, sticky="nsew", padx=20, pady=20)

        self.o2_label = tk.Label(root, text="", font=("Helvetica", 36), fg="cyan", bg='black')
        self.o2_label.grid(row=2, column=1, sticky="nsew", padx=20, pady=20)

        self.update_plot_loop()

    def generate_neurokit_ecg(self, heart_rate, duration=5):
        """Generate ECG signal using NeuroKit."""
        ecg_signal = nk.ecg_simulate(duration=duration, heart_rate=heart_rate, noise=0.01)
        time_range = np.linspace(0, duration, len(ecg_signal))
        return time_range, ecg_signal

    def generate_abp_curve(self, heart_rate, duration=5, waveform_duration=1):
        """Generate synchronized ABP signal based on heart rate, repeating waveform."""
        fs = 100  # Sampling frequency (Hz)
        #t = np.linspace(0, duration, int(fs * duration), endpoint=False)
        beat_interval = 60 / heart_rate  # Interval between beats in seconds
        waveform_duration = beat_interval
        num_beats = int(duration / beat_interval)
        print(beat_interval)

        # Define a base waveform (sine wave)
        single_wave = np.sin(np.linspace(0, 2*np.pi*num_beats, int(fs * duration)))

        # Define a shifted half sine wave
        shifted_half_wave = 2*np.sin(np.linspace(0, 2*np.pi*num_beats, int(fs * duration)))  # Full sine wave
        #shifted_half_wave = np.where(shifted_half_wave<0,np.zeros(shifted_half_wave.shape),shifted_half_wave) # Keep only the positive half
        shifted_half_wave[shifted_half_wave < 0]  = 0 # Keep only the positive half

        #shifted_half_wave = np.concatenate((shifted_half_wave, np.zeros(len(single_wave) - len(shifted_half_wave))))  # Align lengths

        # Shifted half sine wave (1/6 period shift to the left)
        left_shift = int(len(shifted_half_wave) / num_beats/3)
        shifted_wave = np.roll(shifted_half_wave, -left_shift)
        shifted_wave = shifted_wave - 0.5  # Downward shift so its minimum aligns with the sine wave's minimum

        # Combine the two waves
        combined_wave = single_wave + shifted_wave

        # # Repeat the waveform based on heart rate
        # num_beats = int(duration / beat_interval)
        # print(num_beats)
        # abp_signal = np.tile(combined_wave, num_beats)
        
        t = np.linspace(0, duration, int(fs * duration), endpoint=False)
        #abp_signal = abp_signal[:len(t)]  # Truncate to match time length

        print(f"Time vector shape: {t.shape}")
        print(f"ABP signal shape: {combined_wave.shape}")

        return t, combined_wave

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

                # Plot NeuroKit ECG simulation in the first row (neon green)
                if heart_rate > 0:
                    time_range, ecg_signal = self.generate_neurokit_ecg(heart_rate)
                    self.axs[0].plot(time_range + self.times[-1], ecg_signal, 'lime', lw=1.5, label='EKG Pulse')  # Neon green
                    self.axs[0].set_facecolor('black')

                    # Plot synchronized ABP curve in the second row (red)
                    time_range_abp, abp_signal = self.generate_abp_curve(heart_rate)
                    self.axs[1].plot(time_range_abp + self.times[-1], abp_signal, 'red', label='ABP Curve')
                    self.axs[1].set_facecolor('black')
                    self.axs[1].set_ylim(-2, 2)  # Set y-axis limits for the middle graph

                    print(min(abp_signal))

                    # Plot the same curve for O2 saturation in the third row (bright blue)
                    self.axs[2].plot(time_range_abp + self.times[-1], abp_signal*0.6, 'cyan', label='O2 Saturation Curve')  # Bright blue
                    self.axs[2].set_facecolor('black')
                    self.axs[2].set_ylim(-2, 2)  # Set y-axis limits for the middle graph


                # Apply black background and black elements to the plots
                for ax in self.axs:
                    ax.set_facecolor('black')
                    ax.spines['bottom'].set_color('black')  # Change bounding box to black
                    ax.spines['top'].set_color('black')
                    ax.spines['right'].set_color('black')
                    ax.spines['left'].set_color('black')

                    # Set axis ticks and labels to black
                    ax.yaxis.label.set_color('black')
                    ax.xaxis.label.set_color('black')
                    ax.tick_params(axis='y', colors='black')
                    ax.tick_params(axis='x', colors='black')

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
