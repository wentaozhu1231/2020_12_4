class ActualClass {
 public:
  ActualClass(const double&value);
  double get_value()const;
  double add(const double& to_add);
 private:
  double value_;
};
