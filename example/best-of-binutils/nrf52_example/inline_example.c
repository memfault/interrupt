static int prv_compute_power_sum(int count) {
  int result = 0;

  for (int i = 0; i < count; i++) {
    result += i * i;
  }

  return result;
}

int compute_power_sum(int count) {
  if (count < 0) {
    return -1;
  }

  return prv_compute_power_sum(count);
}
