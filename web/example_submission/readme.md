# Data-centric Stock Trading Task
This template includes all the essential source files for the Stock Trading task in the FinRL ICAIF contest. Below, you'll find detailed descriptions of each component.

## Submission
```
├── Finrl-contest 
│ ├── trained_models # Your trained PPO weights
│ ├── test.py # File for testing your submitted PPO model
│ ├── readme.md # File to explain the your code
│ ├── requirements.txt # Have it if adding any new packages
│ ├── And any additional scripts you create
```

## Instruction
To install the necessary dependencies, please ensure you are using Python 3.10 as your interpreter.
```
pip install swig
pip install box2d
pip install git+https://github.com/AI4Finance-Foundation/FinRL.git
```

## Deliverable
We encourage contestants to curate their own dataset. To ensure a smooth testing phase, participants should design their data curation pipeline to seamlessly incorporate previously unseen, hidden test data (provided in a basic format as `train_data.csv`). For instance, we will use `test.py` to assess the performance of your submitted model.

```
python3 test.py --start_date 2022-01-01 --end_date 2022-12-31 --data_file test_data.csv
```
