# Fake Job Posting Detection

An explainable machine learning framework for detecting fraudulent job postings using hybrid classification techniques and SHAP-based interpretability.

## Project Overview

This project implements a multi-model ensemble approach combining traditional ML algorithms (Random Forest, XGBoost) with explainability analysis to identify fake job postings. The system provides both high accuracy and human-interpretable predictions.

**Key Features:**
- Multi-model ensemble classification (XGBoost, Scikit-learn)
- SHAP values for feature importance visualization
- Interactive Streamlit web application
- Real-time prediction with confidence scores
- Comprehensive model evaluation metrics

## Dataset

- **Source:** Fake Job Postings Dataset
- **Features:** Job title, company profile, description, requirements, salary range, etc.
- **Target:** Binary classification (Fake / Legitimate)

## Installation

```bash
pip install -r requirements.txt
```

## Usage

```bash
streamlit run app.py
```

## Project Structure

- `app.py` - Streamlit web application
- `training.ipynb` - Model training & evaluation notebook
- `requirements.txt` - Python dependencies
- `artifacts/` - Trained model files
- `fake_job_postings.zip` - Dataset
- `fake_job_posting_detection.pdf` - Documentation

## Model Architecture

- **XGBoost** (primary classifier)
- **Random Forest** (ensemble)
- **Logistic Regression** (baseline)

## Evaluation Metrics

- Accuracy, Precision, Recall, F1-Score
- ROC-AUC
- SHAP feature importance

## Author

Aakash Gs  
Amrita Vishwa Vidyapeetham, Coimbatore  
B.Tech CSE-AI

## License

Educational purposes.
