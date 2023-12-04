import re

from thirdparty.sdv_testing_tool.sdv_testing_tool.config.config import CONFIG
from thirdparty.sdv_testing_tool.sdv_testing_tool.enums.marks import TC_ID, PrimaryComponent, Priority, Suite, SweLevel, Tasks
from thirdparty.sdv_testing_tool.sdv_testing_tool.helpers.customize_marks import SDVMarks
from thirdparty.sdv_testing_tool.sdv_testing_tool.helpers.helpers import get_utc_time_now
from thirdparty.sdv_testing_tool.sdv_testing_tool.providers.service.logger import SDVLogger

LOGGER = SDVLogger.get_logger("test-OTA_UPDATER")


@SDVMarks.links("TASK_127")
@SDVMarks.add(
    SweLevel.INTEGRATION,
    Priority.P1,
    PrimaryComponent.OTA_UPDATER,
    Suite.SMOKE,
    TC_ID.SDV_00009,
)
def test_ota_positive(iot_bridge_ota_iot_thing):
    ota_service, iot_bridge_service, iot_thing = iot_bridge_ota_iot_thing

    update_qr = ota_service.config.source_conf["update_topic"]

    time_utc_now = get_utc_time_now()

    iot_thing.subscribe_to_topic(update_qr)
    signed_url = iot_thing.get_ota_service_test_package()
    msg_to_send = {"bucketUrl": signed_url}
    iot_thing.publish_message(update_qr, msg_to_send)

    msg = f"\[updater\] Download image: download url {re.escape(signed_url)}"
    assert ota_service.expects_log_message(msg, since=time_utc_now)

    msg = "\[updater\] Download image: done."
    assert ota_service.expects_log_message(msg, since=time_utc_now, timeout=20)

    msg = "\[updater\] Updating running docker container"
    assert ota_service.expects_log_message(msg, since=time_utc_now)
