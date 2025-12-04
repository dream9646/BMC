// Example: Minimal probe() skeleton
static int mydev_probe(struct platform_device *pdev) {
  // 1. Parse device-tree resource
  struct resource *res = platform_get_resource(pdev, IORESOURCE_MEM, 0);

  // 2. Map register
  void *base = devm_ioremap_resource(&pdev->dev, res);

  // 3. Get IRQ
  int irq = platform_get_irq(pdev, 0);

  // 4. Allocate data structure
  struct mydev_data *data =
      devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);

  // 5. Register subsystem
  return devm_request_irq(&pdev->dev, irq, mydev_irq_handler, 0, "mydev",
                          data);
}
